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

/**
 * The MVC View
 *
 * @author: software@arreckx.com
 */
#include "ui_view.hpp"

#include "asx.h"
#include "resource.h"

#include <etl/format_spec.h>
#include <etl/to_string.h>

#include <logger.h>

namespace
{
   const char *const DOM = "ui.view";
}

using namespace rtos::tick;


UIView::UIView( UIModel &model ) : model{ model }
{
   gfx_mono_init();
}

void UIView::draw()
{
   // Clear the screen (init is faster than drawing a rectangle)
   gfx_mono_init();

   draw_box();
   draw_prog();
   draw_walkman();
   draw_counter();
   draw_contact();
   draw_nonc();
}

void UIView::draw_splash()
{
   gfx_mono_put_bitmap( &logo_bm, 0, 0 );
}

void UIView::draw_prog( bool highlight )
{
   // Clear to overwrite
   gfx_mono_draw_filled_rect( 14, 3, 24, 10, GFX_PIXEL_CLR );

   if ( model.get_pgm() < 0 )
   {
      // Draw a back arrow. We draw rather than bitmap given the height
      // Start with setting all on (the arrow can only exists selected)
      gfx_mono_draw_filled_rect( 14, 3, 24, 10, GFX_PIXEL_SET );
      // Draw h bar accross
      gfx_mono_draw_filled_rect( 18, 7, 15, 2, GFX_PIXEL_CLR );
      // Draw arrow head v bar
      gfx_mono_draw_filled_rect( 20, 5, 2, 6, GFX_PIXEL_CLR );
      // Draw right up tail
      gfx_mono_draw_filled_rect( 31, 4, 2, 3, GFX_PIXEL_CLR );
      // Pixel to finsh (line is faster)
      gfx_mono_draw_vertical_line( 19, 6, 4, GFX_PIXEL_CLR );
      gfx_mono_draw_vertical_line( 21, 4, 8, GFX_PIXEL_CLR );
   }
   else
   {
      if ( model.get_pgm() == 0 )
      {
         gfx_mono_draw_string( "MAN", 16, 4, &sysfont );
      }
      else
      {
         etl::string<2>   pgmStr{ "P" };
         etl::format_spec format;

         format.width( 1 ).decimal();
         etl::to_string( model.get_pgm() % 10, pgmStr, format, true );
         gfx_mono_draw_string( pgmStr.c_str(), 20, 4, &sysfont );
      }

      if ( highlight )
      {
         gfx_mono_draw_filled_rect( 14, 3, 24, 10, GFX_PIXEL_XOR );
      }
   }
}

void UIView::draw_contact()
{
   if ( model.contact_is_open() )
   {
      gfx_mono_put_bitmap( &switch_opened_bm, 11, 48 );
   }
   else
   {
      gfx_mono_put_bitmap( &switch_closed_bm, 11, 48 );
   }
}

void UIView::draw_nonc()
{
   gfx_mono_draw_string( model.contact_is_no() ? "NO" : "NC", 35, 52, &sysfont );
}

void UIView::draw_counter()
{
   int32_t          counter = model.get_counter();
   etl::string<5>   cntStr{ "-----" };
   etl::format_spec format;

   format.width( 5 ).fill( '0' ).decimal();

   if ( counter >= 0 )
   {
      if ( counter < 100000 )
      {
         etl::to_string( counter, cntStr, format );
      }
      else
      {
         // Display the number with a + for large cycles
         cntStr == "+";
         etl::to_string( counter % 100000, cntStr, format, true );
      }
   }

   gfx_mono_draw_string( cntStr.c_str(), 15, 36, &sysfont );
}

void UIView::draw_walkman( uint8_t select )
{
   uint8_t x = 13;

   // Need to erase the right hand icon
   auto erase_adjacent = [] { gfx_mono_draw_filled_rect( 29, 17, 13, 13, GFX_PIXEL_CLR ); };

   switch ( model.get_state() )
   {
   case UIModel::program_state_t::paused:
      x = 11;  // Change to highlight
      gfx_mono_put_bitmap( &rec_play_bm, x, 16 );
      gfx_mono_put_bitmap( &rec_stop_bm, x + 16, 16 );
      break;
   case UIModel::program_state_t::running:
      gfx_mono_put_bitmap( &rec_pause_bm, x, 16 );
      erase_adjacent();
      break;
   case UIModel::program_state_t::stopped:
      gfx_mono_put_bitmap( &rec_play_bm, x, 16 );
      erase_adjacent();
      break;
   default: break;
   }

   // Inverse the selected one
   if ( select )
   {
      gfx_mono_draw_filled_rect( x + ( select == 1 ? 3 : 19 ), 18, 11, 11, GFX_PIXEL_XOR );
   }
}

void UIView::draw_box()
{
   gfx_mono_draw_rect( 0, 0, 48, 64, GFX_PIXEL_SET );

   // Dot the lines
   for ( gfx_coord_t x = 2; x < 48; x += 2 )
   {
      gfx_mono_draw_pixel( x, 15, GFX_PIXEL_SET );
      gfx_mono_draw_pixel( x, 31, GFX_PIXEL_SET );
      gfx_mono_draw_pixel( x, 47, GFX_PIXEL_SET );
   }
}

void UIView::draw_cursor( uint8_t pos )
{
   gfx_coord_t y = pos * 16 + 3;

   gfx_mono_draw_line( 3, y, 9, y + 4, GFX_PIXEL_SET );
   gfx_mono_draw_line( 9, y + 4, 3, y + 8, GFX_PIXEL_SET );
   gfx_mono_draw_vertical_line( 3, y, 8, GFX_PIXEL_SET );
}

void UIView::erase_curson_hint( uint8_t pos )
{
   gfx_coord_t y = pos * 16 + 5;

   gfx_mono_draw_vertical_line( 2, y, 4, GFX_PIXEL_CLR );
}

void UIView::draw_curson_hint( uint8_t pos )
{
   gfx_coord_t y = pos * 16 + 5;

   gfx_mono_draw_vertical_line( 2, y, 4, GFX_PIXEL_SET );
}

void UIView::erase_cursor( uint8_t pos )
{
   gfx_coord_t y = pos * 16 + 3;
   gfx_mono_draw_filled_rect( 3, y, 7, 9, GFX_PIXEL_CLR );
}

void UIView::draw_program_setup_dialog()
{
   // Clear the screen
   gfx_mono_init();

   // Draw the time for close
   gfx_mono_draw_string( "T:", 4, 4, &sysfont );

   // Draw the intersecting line
   gfx_mono_draw_line( 0, 30, 48, 30, GFX_PIXEL_SET );
   gfx_mono_draw_line( 0, 31, 48, 31, GFX_PIXEL_SET );

   // Draw the time for the open
   gfx_mono_draw_string( "T:", 4, 32 + 4, &sysfont );
   gfx_mono_put_bitmap( &switch_opened_bm, 16, 32 );

   // Draw the dotted lines
   for ( gfx_coord_t x = 2; x < 48; x += 2 )
   {
      gfx_mono_draw_pixel( x, 15, GFX_PIXEL_SET );
      gfx_mono_draw_pixel( x + 1, 15, GFX_PIXEL_CLR );
      gfx_mono_draw_pixel( x, 47, GFX_PIXEL_SET );
      gfx_mono_draw_pixel( x + 1, 47, GFX_PIXEL_CLR );
   }

   // Draw the ':' for the minutes and seconds
   gfx_mono_draw_string( ":", 24, 20, &sysfont );
   gfx_mono_draw_string( ":", 24, 52, &sysfont );

   manual_program_draw_digit( first_highlight, 0, 0 );
   manual_program_draw_digit( first_normal, 0, 1 );
   manual_program_draw_digit( first_normal, 1, 0 );
   manual_program_draw_digit( first_normal, 1, 1 );
}

/**
 * Draw the manual program selection digits (on/off/min/sec)
 * @param show 0 : Normal draw
 *             1 : Highlight the digit, from a normal draw
 *             2 : Draw the digit selected
 *             3 : Restore the digit to normal view
 */
void UIView::manual_program_draw_digit( show_digit_t show, uint8_t row, uint8_t column )
{
   uint8_t                  x              = 8 + 24 * column;
   uint8_t                  y              = 20 + 32 * row;
   static constexpr uint8_t MINUTES_COLUMN = 0;
   static constexpr uint8_t ON_ROW         = 0;
   etl::string<2>           digits;
   etl::format_spec         format;

   // Clear the area
   if ( show == back_to_normal )
   {
      gfx_mono_draw_filled_rect( x - 2, y - 2, 15, 11, GFX_PIXEL_CLR );
   }

   uint8_t value = ( row == ON_ROW )
                      ?
                      /* ON  */ ( column == MINUTES_COLUMN ? model.on_min : model.on_sec )
                      :
                      /* OFF */ ( column == MINUTES_COLUMN ? model.off_min : model.off_sec );

   LOG_INFO(
      DOM, "Draw digit Col:%d Row:%d = %s%d%s", column, row, show ? ">>" : "", value,
      show ? "<<" : "" );


   format.width( 2 ).fill( '0' );
   gfx_mono_draw_string( etl::to_string( value, digits, format ).c_str(), x, y, &sysfont );

   // Inverse the digits
   if ( show == first_highlight )
   {
      // Draw the frame too
      gfx_mono_draw_filled_rect( x - 2, y - 2, 15, 11, GFX_PIXEL_XOR );
   }
   else if ( show == next_highlight )
   {
      // Just the character
      gfx_mono_draw_filled_rect( x, y, 12, 7, GFX_PIXEL_XOR );
   }
}

void UIView::draw_usb()
{
   gfx_mono_put_bitmap( &usb_symbol_bm, 0, 0 );
}
