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
/*
 * console_task.cpp
 *
 * Created: 28/08/2021 20:27:07
 *  Author: software@arreckx.com
 */
#include "console.hpp"

#include "asx.h"
#include "parser.hpp"
#include "trace.h"

#include <fx.hpp>

#include <logger.h>

#include "console_server.hpp"
#include "msg_defs.hpp"
#include "program_manager.hpp"

using rtos::tick_t;

// USB static management
namespace
{
   const char *const DOM = "console";

   auto cdc_available_semaphore = rtos::BinarySemaphore{};
   bool cdc_transfert_allowed{ false };
   bool first_time{ true };
   bool usb_mode{ false };
}  // namespace

//
// Hooks for the CDC interface
//
extern "C" bool console_cdc_enabled( uint8_t port )
{
   UNUSED( port );
   cdc_transfert_allowed = true;
   cdc_available_semaphore.give_from_isr( nullptr );
   trace_set( TRACE_INFO );

   return true;
}

extern "C" void console_cdc_disabled( uint8_t port )
{
   UNUSED( port );
   cdc_transfert_allowed = false;
   trace_clear( TRACE_INFO );
}

void console_putc( vt100::char_t c )
{
   LOG_DEBUG( DOM, "PUT %c [0x%.2x]", isalnum( c ) ? c : '.', c );
   udi_cdc_multi_putc( 0, (int)c );
}

/** Create the timer for the splash */
Console::Console( ProgramManager &program_manager )
   : parser{ temp_program, error_buffer }
   , program_manager{ program_manager }
   , task( etl::delegate<void()>::create<Console, &Console::run>( *this ) )
{}

void Console::show_error()
{
   using T = TTerminal;

   uint8_t err_position = parser.get_error_position();

   // Display the error message
   T::putc( '#' );

   // Insert spaces
   for ( uint8_t i = 0; i <= err_position; ++i )
   {
      T::putc( ' ' );
   }

   T::putc( '^' );
   T::move_to_start_of_next_line();
   print_error( error_buffer.c_str(), false );
}

void Console::print_error( const char error[], bool is_pgm_str )
{
   using T = TTerminal;

   T::putc( '#' );
   T::putc( ' ' );

   if ( is_pgm_str )
   {
      T::print_P( error );
   }
   else
   {
      T::puts( error );
   }


   T::move_to_start_of_next_line();
}

/**
 * The task entry point
 */
void Console::run()
{
   using T = TTerminal;

   while ( true )
   {
      optional_buffer_view_t v;

      // Wait for the cdc to become available
      if ( ! cdc_transfert_allowed )
      {
         server.reset();
         cdc_available_semaphore.take();
      }

      if ( first_time )
      {
         T::print_P( PSTR( "# Welcome to Cyclo! Type 'help' for help.\r\n" ) );
         T::move_to_start_of_next_line();
         first_time = false;
      }

      server.print_prompt();

      while ( ! v )
      {
         v = server.process_input( (vt100::char_t)udi_cdc_getc() );

         rtos::delay( 1 );

         if ( ! cdc_transfert_allowed )
         {
            break;
         }
      }

      // Process the line
      if ( v )
      {
         process( *v );
      }
   }
}

void Console::process( etl::string_view line )
{
   auto res = parser.parse( line );

   switch ( res )
   {
   case Parser::Result::nothing: break;
   case Parser::Result::error: show_error(); break;
   case Parser::Result::program:
      if ( ! usb_mode )
      {
         usb_mode = true;
         fx::publish( msg::USBConnected{} );
      }

      program_manager.load( temp_program );
      // Copy this program, so it can be saved as is
      last_program.clear();
      last_program.assign( line.begin(), line.end() );
      break;
   case Parser::Result::help: show_help(); break;
   case Parser::Result::list:
      program_manager.scan();
      show_list();
      break;
   case Parser::Result::quit:
      usb_mode = false;
      fx::publish( msg::StopProgram{} );
      fx::publish( msg::USBDisconnected{} );

      break;
   case Parser::Result::del: program_manager.erase( parser.get_program_number() ); break;
   case Parser::Result::run:
      // Toggle USB mode
      if ( ! usb_mode )
      {
         usb_mode = true;
         fx::publish( msg::USBConnected{} );
      }

      // Check the program exists
      if ( program_manager.get_map()[ parser.get_program_number() ] )
      {
         auto pgmNumber = parser.get_program_number();
         program_manager.load( pgmNumber );
         program_manager.set_lastused( pgmNumber );

         fx::publish( msg::StartProgram{ true } );
      }
      else
      {
         print_error( PSTR( "No such program number" ) );
      }
      break;
   case Parser::Result::save:
      if ( last_program.empty() )
      {
         print_error( PSTR( "No valid program to save" ) );
      }
      else
      {
         auto pgmNumber = parser.get_program_number();

         // Store
         program_manager.write_pgm_at( pgmNumber, last_program );

         // Make it the last used
         program_manager.set_lastused( pgmNumber );
      }

      break;
   case Parser::Result::autostart:
   {
      auto pgm = parser.get_program_number();

      // Make sure the program exists
      if ( pgm >= 0 and not program_manager.get_map()[ pgm ] )
      {
         print_error( PSTR( "Empty program slot" ) );
      }
      else
      {
         program_manager.set_autostart( pgm );
      }
   }
   break;
   default: LOG_ERROR( DOM, "Unexpected" ); break;
   }
}

void Console::show_help()
{
   auto help = PSTR(
      "Commands are separated with spaces. All commands can be shortened to just 1 letter.\r\n"
      "\r\n"
      "Basic control commands:\r\n"
      "  open  .. Opens the contact\r\n"
      "  close .. Closes the contact\r\n"
      "\r\n"
      "A delay is an unsigned integer (32bits) followed by an optional unit:\r\n"
      "  H .. hour\r\n"
      "  M .. minutes\r\n"
      "  s .. seconds (default if no unit)\r\n"
      "  m .. milliseconds\r\n"
      "\r\n"
      "A valid program is a series of open/close/delay.\r\n"
      "The system enforces a minimum delay of 1s between contact state changes.\r\n"
      "\r\n"
      "Example:\r\n"
      "  o 1 500m c 1H 30M\r\n"
      "To loop a sequence, add '*' at the end.\r\n"
      "  close 250m open 10 c o *\r\n"
      "\r\n"
      "Other commands:\r\n"
      "  list           : List saved programs\n\r"
      "  save [1-9]     : Save the last valid program\r\n"
      "  del [1-9]      : Delete the program at the given location\r\n"
      "  run [0-9]      : Run the given program\r\n"
      "  auto [0-9|off] : Start the program automatically on power-up - or turn off\r\n"
      "  quit           : Leave this shell and re-enable manual mode\r\n"
      "Fast run:\r\n"
      "  [0-9]          : Type a valid program number to run it.\r\n" );

   TTerminal::print_P( help );
}

void Console::show_list()
{
   // Iterate the bitset
   int8_t index, next_index = 0;
   auto   autostart_index = program_manager.get_autostart_index();

   do
   {
      index      = next_index;
      next_index = program_manager.get_next( index );

      // Print the index
      TTerminal::move_forward();
      TTerminal::putc( autostart_index == index ? '*' : ' ' );
      TTerminal::move_forward( 2 );
      TTerminal::putc( '0' + index );
      TTerminal::putc( ':' );
      TTerminal::move_forward( 4 );
      TTerminal::puts( program_manager.get_pgm( index ) );
      TTerminal::move_to_start_of_next_line();
   } while ( next_index > 0 );
}
