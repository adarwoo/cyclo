/*
 * ui_view.cpp
 *
 * Created: 23/08/2021 23:21:04
 *  Author: micro
 */
#include "ui_view.hpp"

#include "asx.h"

#include <logger.h>

namespace
{
   const char *const DOM = "ui.view";
}

using namespace rtos::tick;


const gfx_mono_color_t PROGMEM logo_header[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xf,
   0x7,  0x7,  0x3,  0x1,  0x1,  0x3,  0x3,  0x3,  0x1,  0x1,  0x1,  0x3,  0x3,  0xf,  0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x3f, 0x3f, 0x1f, 0x3,  0x0,  0x0,  0x0,
   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xfc, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x3f,
   0x1f, 0x1f, 0x7,  0x3,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
   0x0,  0xf0, 0xe0, 0xe0, 0xc0, 0x80, 0x80, 0x0,  0x3,  0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3,  0x1,  0x0,  0x0,  0x0,  0xf8,
   0xfc, 0x8e, 0x87, 0x87, 0x87, 0x8f, 0xfe, 0xfc, 0x0,  0x0,  0x0,  0x0,  0x0,  0x80, 0xc0, 0x0,
   0x0,  0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xc7, 0xf1, 0xf1, 0xf0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x0,  0x0,  0x0,  0x0,  0xf,
   0xf,  0x1,  0x1,  0x1,  0x1,  0x1,  0xf,  0xf,  0x0,  0x0,  0x0,  0x7c, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xfc, 0xfc, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x0,  0x0,  0x0,  0x0,  0x80, 0xe0,
   0xf8, 0xf8, 0xf8, 0xf8, 0xe0, 0x0,  0x0,  0x0,  0x0,  0xc0, 0xf0, 0xfe, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3,  0x0,  0x0,  0x0,  0xfe, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x80, 0x0,  0x0,  0x0,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0x8f, 0x87, 0xc0, 0xe0, 0xc0, 0x80, 0x87, 0xc7, 0xc7,
   0xcf, 0xef, 0xdf, 0xe7, 0xe3, 0xe0, 0xf0, 0xf0, 0xf1, 0xe1, 0xe3, 0xe3, 0xf7, 0xf7, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

const gfx_mono_color_t PROGMEM switch_opened[] = {
   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80, 0xc0, 0x60, 0x30, 0x18, 0xc,  0x4,
   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x84, 0x84, 0x84, 0x84, 0x8e, 0x9b, 0x91, 0x9b,
   0x8f, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x8e, 0x9b, 0x91, 0x9b, 0x8e, 0x84, 0x84, 0x84, 0x84,
};

const gfx_mono_color_t PROGMEM switch_closed[] = {
   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80, 0x80, 0x80, 0x80, 0x80, 0xc0,
   0xc0, 0xc0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x84, 0x84, 0x84, 0x84, 0x8e, 0x9b, 0x91, 0x9b,
   0x8f, 0x83, 0x83, 0x81, 0x81, 0x81, 0x81, 0x8f, 0x9b, 0x91, 0x9b, 0x8e, 0x84, 0x84, 0x84, 0x84,
};

const gfx_mono_color_t PROGMEM rec_stop[] = {
   0x0, 0x0,  0xfe, 0x2,  0x2,  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0x2,  0x2,  0xfe, 0x0,
   0x0, 0x80, 0x3f, 0xa0, 0x20, 0xa7, 0x27, 0xa7, 0x27, 0xa7, 0x27, 0xa7, 0x20, 0xa0, 0x3f, 0x80,
};

const gfx_mono_color_t PROGMEM rec_play[] = {
   0x0, 0x0,  0xfe, 0x2,  0x2,  0xf2, 0xf2, 0xe2, 0xe2, 0xc2, 0xc2, 0x82, 0x2,  0x2,  0xfe, 0x0,
   0x0, 0x80, 0x3f, 0xa0, 0x20, 0xa7, 0x27, 0xa3, 0x23, 0xa1, 0x21, 0xa0, 0x20, 0xa0, 0x3f, 0x80,
};

const gfx_mono_color_t PROGMEM rec_pause[] = {
   0x0, 0x0,  0xfe, 0x2,  0x2,  0xf2, 0xf2, 0x2,  0x2,  0x2,  0xf2, 0xf2, 0x2,  0x2,  0xfe, 0x0,
   0x0, 0x80, 0x3f, 0xa0, 0x20, 0xa7, 0x27, 0xa0, 0x20, 0xa0, 0x27, 0xa7, 0x20, 0xa0, 0x3f, 0x80,
};

const gfx_mono_color_t PROGMEM usb_symbol[] = {
   0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
   0x0,  0x0,  0x0,  0x0,  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xe0, 0xe0, 0xe0,
   0xe0, 0xc0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
   0x0,  0xc0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xf8, 0xf0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xf0,
   0xf8, 0xfc, 0xde, 0xcf, 0xc7, 0xc3, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc3, 0xc7, 0xc7, 0xc7,
   0xc7, 0xc3, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xf8, 0xf0, 0xf0, 0xe0, 0xc0, 0xc0, 0x0,  0x0,
   0x0,  0x0,  0x3,  0x7,  0xf,  0xf,  0xf,  0xf,  0x7,  0x3,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
   0x0,  0x0,  0x0,  0x1,  0x3,  0x7,  0xf,  0x1e, 0x3c, 0x78, 0x70, 0x60, 0x60, 0x60, 0x60, 0x60,
   0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x0,  0x0,  0x7,  0x3,  0x3,  0x1,  0x0,  0x0,  0x0,  0x0,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};

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
   struct gfx_mono_bitmap logo_bm = {
      .width = 48, .height = 64, .type = GFX_MONO_BITMAP_PROGMEM, (gfx_mono_color_t *)logo_header };

   gfx_mono_put_bitmap( &logo_bm, 0, 0 );
}

void UIView::draw_prog( bool highlight )
{
   if ( model.get_pgm() == 0 )
   {
      gfx_mono_draw_string( "MAN", 16, 4, &sysfont );
   }
   else
   {
      char pgmString[ 4 ];
      snprintf( pgmString, 3, "P%1u", model.get_pgm() % 10 );
   }

   if ( highlight )
   {
      gfx_mono_draw_filled_rect( 14, 3, 24, 10, GFX_PIXEL_XOR );
   }
}

void UIView::draw_contact()
{
   auto *switch_pixmap = model.contact_is_open() ? (gfx_mono_color_t *)switch_opened
                                                 : (gfx_mono_color_t *)switch_closed;

   struct gfx_mono_bitmap sw_bm = {
      .width = 24, .height = 16, .type = GFX_MONO_BITMAP_PROGMEM, switch_pixmap };

   gfx_mono_put_bitmap( &sw_bm, 11, 48 );
}

void UIView::draw_nonc()
{
   gfx_mono_draw_string( model.contact_is_no() ? "NO" : "NC", 35, 52, &sysfont );
}

void UIView::draw_counter()
{
   char counterString[] = "-----";

   if ( model.get_state() != UIModel::program_state_t::stopped )
   {
      snprintf( counterString, sizeof( counterString ), "%.5u", model.get_counter() );
   }

   gfx_mono_draw_string( counterString, 15, 36, &sysfont );
}

void UIView::draw_walkman( uint8_t select )
{
   struct gfx_mono_bitmap _bm = {
      .width = 16, .height = 16, .type = GFX_MONO_BITMAP_PROGMEM, (gfx_mono_color_t *)rec_play };

   uint8_t x = 11;

   // Need to erase the right hand icon
   auto erase_adjacent = [] { gfx_mono_draw_filled_rect( 27, 17, 13, 13, GFX_PIXEL_CLR ); };

   switch ( model.get_state() )
   {
   case UIModel::program_state_t::paused:
      x                = 9;
      _bm.data.progmem = (gfx_mono_color_t *)rec_play;
      gfx_mono_put_bitmap( &_bm, x, 16 );
      _bm.data.progmem = (gfx_mono_color_t *)rec_stop;
      gfx_mono_put_bitmap( &_bm, 25, 16 );
      break;
   case UIModel::program_state_t::running:
      _bm.data.progmem = (gfx_mono_color_t *)rec_pause;
      gfx_mono_put_bitmap( &_bm, x, 16 );
      erase_adjacent();
      break;
   case UIModel::program_state_t::stopped:
      _bm.data.progmem = (gfx_mono_color_t *)rec_play;
      gfx_mono_put_bitmap( &_bm, x, 16 );
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
   gfx_mono_draw_line( 3, y + 8, 3, y, GFX_PIXEL_SET );
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

   // Draw all
   auto *open_pixmap   = (gfx_mono_color_t *)switch_opened;
   auto *closed_pixmap = (gfx_mono_color_t *)switch_closed;

   struct gfx_mono_bitmap sw_bm = {
      .width = 24, .height = 16, .type = GFX_MONO_BITMAP_PROGMEM, closed_pixmap };

   // Draw the time for close
   gfx_mono_draw_string( "T:", 4, 4, &sysfont );
   gfx_mono_put_bitmap( &sw_bm, 16, 0 );

   // Draw the intersecting line
   gfx_mono_draw_line( 0, 30, 48, 30, GFX_PIXEL_SET );
   gfx_mono_draw_line( 0, 31, 48, 31, GFX_PIXEL_SET );

   // Draw the time for the open
   sw_bm.data.progmem = open_pixmap;
   gfx_mono_draw_string( "T:", 4, 32 + 4, &sysfont );
   gfx_mono_put_bitmap( &sw_bm, 16, 32 );

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
   char                     buffer[ 4 ];
   uint8_t                  x              = 8 + 24 * column;
   uint8_t                  y              = 20 + 32 * row;
   static constexpr uint8_t MINUTES_COLUMN = 0;
   static constexpr uint8_t ON_ROW         = 0;

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

   snprintf( buffer, sizeof( buffer ), "%.2u", value );
   gfx_mono_draw_string( buffer, x, y, &sysfont );

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
   struct gfx_mono_bitmap usb_bm = {
      .width = 48, .height = 32, .type = GFX_MONO_BITMAP_PROGMEM, (gfx_mono_color_t *)usb_symbol };

   gfx_mono_put_bitmap( &usb_bm, 0, 0 );
}