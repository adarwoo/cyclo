/******************************************************************************
The MIT License(MIT)
https://github.com/adarwoo/cyclo

Copyright(c) 2021 Guillaume ARRECKX - software@arreckx.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include <cassert>
#include <cstdlib>
#include <thread>

// For the getch
#include <logger.h>
#include <termios.h>
#include <unistd.h>

// For the logger
#include "asx.h"
#include "keypad.h"

#include <rtos.hpp>

extern "C" void nvm_init( void );
extern "C" bool console_cdc_enabled( uint8_t port );

namespace
{
   // Logger domain
   const char *const DOM = "sim";

   // Synchronous queue to send to the UDC
   rtos::Queue<char, 16> key_queue;
}  // namespace

extern "C"
{
   uint8_t ports[ 8 ] = { 0 };

   void board_init()
   {
      nvm_init();
      console_cdc_enabled( 0 );
   }

   bool ioport_get_pin_level( ioport_pin_t pin )
   {
      uint8_t bitpos     = pin & 0b111;
      uint8_t bitmask    = 1 << bitpos;
      uint8_t port       = pin >> 8;
      char    portLetter = 'A' + port;

      bool retval = ports[ port ] & bitmask;

      LOG_TRACE( DOM, "PORT%c,%d : %s", portLetter, bitpos, retval ? "on" : "off" );

      return retval;
   }

   void ioport_set_pin_level( ioport_pin_t pin, bool level )
   {
      uint8_t bitpos     = pin & 0b111;
      uint8_t bitmask    = 1 << bitpos;
      uint8_t port       = pin >> 8;
      char    portLetter = 'A' + port;

      bool from = ports[ port ] & bitmask;

      LOG_TRACE(
         DOM, "PORT%c,%d %s > %s", portLetter, bitpos, from ? "on" : "off", level ? "ON" : "OFF" );

      if ( level )
      {
         // Mask with value
         ports[ port ] |= bitmask;
      }
      else
      {
         // Clear the bit
         ports[ port ] &= ~bitmask;
      }
   }

   void ioport_toggle_pin_level( ioport_pin_t pin )
   {
      uint8_t bitpos     = pin & 0b111;
      uint8_t bitmask    = 1 << bitpos;
      uint8_t port       = pin >> 8;
      char    portLetter = 'A' + port;

      LOG_TRACE(
         DOM, "PORT%c,%d %s", portLetter, bitpos,
         ports[ port ] & bitmask ? "on > OFF" : "off > ON" );

      // Toggle the bit
      ports[ port ] ^= bitmask;
   }

   //
   // Trace
   //
   void trace_assert( bool test, ioport_pin_t pin ) { assert( test ); }

   //
   // Keypad
   //
   struct keypad_key_t
   {
      uint8_t          mask;     ///< ID of the key
      keypad_handler_t handler;  ///< Callback to call on a push or repeat
      void *           param;    ///< Additonal param to pass along
   };

   // Register a timer for sampling the keypad_pins
   constexpr ioport_pin_t keypad_pins[]         = { KEYPAD_PINS };
   constexpr uint8_t      KEYPAD_NUMBER_OF_KEYS = sizeof( keypad_pins ) / sizeof( ioport_pin_t );

   static keypad_key_t keypad_keys[ KEYPAD_NUMBER_OF_KEYS ] = {
      { KEY_UP }, { KEY_DOWN }, { KEY_SELECT } };

   char getch()
   {
      char           buf = 0;
      struct termios old = { 0 };

      tcgetattr( 0, &old );
      old.c_lflag &= ~ICANON;
      old.c_lflag &= ~ECHO;
      old.c_cc[ VMIN ]  = 1;
      old.c_cc[ VTIME ] = 0;
      tcsetattr( 0, TCSANOW, &old );
      read( 0, &buf, 1 );
      old.c_lflag |= ICANON;
      old.c_lflag |= ECHO;
      tcsetattr( 0, TCSADRAIN, &old );

      return buf;
   }

   void scan_keys()
   {
      LOG_TRACE( DOM, "Task key scanning running" );

      char c           = 0;
      bool send_to_cdc = true;

      while ( c != 'q' )
      {
         uint8_t key = 0;

         if ( c == '/' )
         {
            send_to_cdc = ! send_to_cdc;
            c           = 0;
            continue;
         }

         if ( send_to_cdc )
         {
            if ( c != 0 )
               key_queue.send( c );
         }
         else
         {
            switch ( c )
            {
            case 'w': key = KEY_UP; break;
            case 's': key = KEY_DOWN; break;
            case '\n': key = KEY_SELECT; break;
            default: break;
            }

            if ( key != 0 )
            {
               LOG_DEBUG(
                  DOM, "Handling %s", key == KEY_UP ? "UP" : key == KEY_DOWN ? "DOWN" : "SELECT" );

               for ( size_t i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i )
               {
                  if ( keypad_keys[ i ].mask & key )
                  {
                     // Call the handler
                     keypad_keys[ i ].handler( key, keypad_keys[ i ].param );
                  }
               }
            }
         }

         c = getch();
      }

      exit( 0 );
   }

   /** Initialise the keypad library */
   void keypad_init( void )
   {
      // Start a thread which scan the keypad and calls the callback
      static auto keypad = rtos::Task<typestring_is( "keypad" )>();
      keypad.run( scan_keys );
   }

   // UDI
   int udi_cdc_multi_putc( uint8_t port, int value )
   {
      UNUSED( port );
      putchar( value );
      fflush( stdout );
      return 0;
   }

   int udi_cdc_getc( void )
   {
      char c;
      key_queue.receive( c );
      LOG_DEBUG( DOM, "Sending %#.2x", c );

      return (int)c;
   }

   /**
    * Regsiter a callback for one or many key
    * key_masks Mask of the keys to handle
    * handler Callback function (from isr)
    * param Optional data to pass along
    */
   void keypad_register_callback( uint8_t key_masks, keypad_handler_t handler, void *param )
   {
      uint8_t i;

      for ( i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i )
      {
         if ( keypad_keys[ i ].mask &= key_masks )
         {
            keypad_keys[ i ].handler = handler;
            keypad_keys[ i ].param   = param;
         }
      }
   }
}  // extern "C"
