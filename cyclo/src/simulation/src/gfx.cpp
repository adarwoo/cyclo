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
#include <logger.h>
#include "asx.h"

namespace
{
   // Logger domain
   const char *const GFX = "gfx";
   const char *const GFX_LOW = "gfx.low";
}

//
// GFX library
//
extern "C"
{
   // Dummy font instance
   struct font sysfont;

   void gfx_mono_init() {}

   void gfx_mono_draw_horizontal_line(
      gfx_coord_t x, gfx_coord_t y, gfx_coord_t length, enum gfx_mono_color color )
   {
      LOG_HEADER( GFX_LOW );
      LOG_INFO( GFX_LOW, "*** HLINE @[%2d, %2d] len: %2d", x, y, length );
   }

   void gfx_mono_draw_filled_rect(
      gfx_coord_t         x,
      gfx_coord_t         y,
      gfx_coord_t         width,
      gfx_coord_t         height,
      enum gfx_mono_color color )
   {
      LOG_HEADER( GFX_LOW );
      if ( height == 0 )
      {
         /* Nothing to do. Move along. */
         return;
      }

      while ( height-- > 0 )
      {
         gfx_mono_draw_horizontal_line( x, y + height, width, color );
      }
   }

   void gfx_mono_draw_line(
      gfx_coord_t x1, gfx_coord_t y1, gfx_coord_t x2, gfx_coord_t y2, enum gfx_mono_color color )
   {
      LOG_HEADER( GFX_LOW );
      LOG_INFO( GFX_LOW, "*** LINE [%2d, %2d] [%2d, %2d]", x1, y1, x2, y2 );
   }

   void gfx_mono_put_bitmap( struct gfx_mono_bitmap *bitmap, gfx_coord_t x, gfx_coord_t y )
   {
      LOG_HEADER( GFX );
      LOG_INFO( GFX, "*** BITMAP %p [%2d, %2d]", bitmap, x, y );
   }

   void gfx_mono_draw_rect(
      gfx_coord_t         x,
      gfx_coord_t         y,
      gfx_coord_t         width,
      gfx_coord_t         height,
      enum gfx_mono_color color )
   {
      LOG_HEADER( GFX_LOW );
      LOG_INFO( GFX_LOW, "*** RECT @[%2d, %2d] W[%2d, %2d]", x, y, width, height );
   }

   void gfx_mono_draw_pixel( gfx_coord_t x, gfx_coord_t y, gfx_coord_t color )
   {
      LOG_HEADER( GFX_LOW );
      LOG_INFO( GFX_LOW, "*** PIXEL [%2d, %2d]", x, y );
   }

   void
      gfx_mono_draw_string( const char *str, gfx_coord_t x, gfx_coord_t y, const struct font *font )
   {
      LOG_HEADER( GFX );
      LOG_INFO( GFX, "*** TEXT [%2d, %2d] %s", x, y, str );
   }
}