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
#ifndef VT100_H
#define VT100_H

#include <cstddef>
#include <cstdint>

#include "asx.h"


/** ASCII constants */
namespace ascii
{
   constexpr auto bel = '\x07';
   constexpr auto bs  = '\x08';
   constexpr auto cr  = '\x0d';
   constexpr auto lf  = '\x0a';
   constexpr auto esc = '\x1b';
   constexpr auto del = '\x7f';
}  // namespace ascii

/** Arrow control constants */
namespace vt100
{
   using char_t = decltype( 'x' );
   using putc_t = void ( * )( char_t );

   namespace arrow
   {
      constexpr auto up    = 'A';
      constexpr auto down  = 'B';
      constexpr auto right = 'C';
      constexpr auto left  = 'D';
      constexpr auto end   = 'F';
      constexpr auto home  = 'H';
   }  // namespace arrow

   namespace attr
   {
      // constants/macros/typedefs
      // text attributes
      constexpr auto attr_off    = '\0';
      constexpr auto bold        = '\1';
      constexpr auto uscore      = '\4';
      constexpr auto blink       = '\5';
      constexpr auto reverse     = '\7';
      constexpr auto bold_off    = '\21';
      constexpr auto uscore_off  = '\24';
      constexpr auto blink_off   = '\25';
      constexpr auto reverse_off = '\27';
   }  // namespace attr

   template<putc_t TPutc>
   struct Terminal
   {
      static void init()
      {
         // initializes terminal to "power-on" settings - ESC c
         print_P( PSTR( "\x1b\x63" ) );
      }

      static void clear( void )
      {
         // ESC [ 2 J
         print_P( PSTR( "\x1b[2J" ) );
      }

      static void set_cursor_mode( bool visible )
      {
         print_P( visible ? PSTR( "\x1B[?25h" ) : PSTR( "\x1B[?25l" ) );
      }

      static void putc( char_t c ) { TPutc( c ); }

      static void print_P( const char_t str[] )
      {
         // check to make sure we have a good pointer
         if ( str )
         {
            char c;

            while ( (c = pgm_read_byte( str++ )) )
            {
               TPutc( c );
            }
         }
      }

      static void puts( const char_t *str )
      {
         // check to make sure we have a good pointer
         if ( str )
         {
            char c;

            while ( (c = *str++ ) )
            {
               TPutc( c );
            }
         }
      }

      static inline void ring_bell() { putc( ascii::bel ); }

      static inline void move_to_start_of_next_line()
      {
         putc( ascii::cr );
         putc( ascii::lf );
      }

      static inline void move_back( size_t distance = 1 )
      {
         for ( size_t i = 0; i < distance; ++i )
         {
            putc( ascii::bs );
         }
      }

      static inline void move_forward( size_t distance = 1 )
      {
         for ( size_t i = 0; i < distance; ++i )
         {
            putc( ascii::esc );
            putc(  '[' );
            putc( vt100::arrow::right );
         }
      }

      static inline void move_back_with_erase()
      {
         putc( ascii::bs );
         putc( ' ' );
         putc( ascii::bs );
      }

      static inline void move_to_start() { putc( ascii::cr ); }

      static void erase_right( size_t count )
      {
         size_t i;

         for ( i = 0; i < count; ++i )
         {
            putc( ' ' );
         }

         for ( i = 0; i < count; ++i )
         {
            putc( ascii::bs );
         }
      }
   };
}  // namespace vt100

#endif