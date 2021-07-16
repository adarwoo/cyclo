#ifndef ui_worker_hpp__included
#define ui_worker_hpp__included
/*
 * UI Woker
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "asf.h"

#include "fx.hpp"
#include "msg_defs.hpp"
#include "etl/limits.h"

#include <cstdio>

using namespace rtos::tick;

const gfx_mono_color_t PROGMEM cursor_bm_data[] = {
   0b00000000,
   0b11111111,
   0b11111111,
   0b01111110,
   0b01111110,
   0b00111100,
   0b00011000,
   0b00000000
};

static inline struct gfx_mono_bitmap cursor_bm = {
   .width = 8,
   .height = 8,
   .type=GFX_MONO_BITMAP_PROGMEM,
   (gfx_mono_color_t*)cursor_bm_data
};


enum class quadrant_t : uint8_t
{
   none,
   prog = 1,
   activity,
   counter,
   info,
   stop
};

uint8_t operator*(const quadrant_t q)
   { return static_cast<uint8_t>(q) - 1; }

inline quadrant_t operator++(quadrant_t &q, int)
{
   quadrant_t retval = q;
   uint8_t iq = static_cast<uint8_t>(q);
   q = static_cast<quadrant_t>(++iq);
   
   if (q == quadrant_t::stop)
   {
      q = quadrant_t::prog;
   }
   
   return retval; 
}

inline quadrant_t operator--(quadrant_t &q, int)
{
   quadrant_t retval = q;
   uint8_t iq = static_cast<uint8_t>(q);
   q = static_cast<quadrant_t>(--iq);
   
   if (q == quadrant_t::none)
   {
      q = quadrant_t::info;
   }
   
   return retval;
}


enum class pgm_t : uint8_t
{
   PUSB,
   P1,
   P2,
   P3,
   P4,
   P5,
   P6,
   P7,
   P8,
   P9,
   STOP
};

using cycle_counter_t = uint16_t;

enum class contact_cfg_t : uint8_t
{
   tdb,
   no,
   nc
};

enum class contact_state_t : uint8_t
{
   opened,
   closed
};


// On UI timer
//
class UIWorker : public fx::Worker<UIWorker, fx::DispatcherStarted, msg::Keypad, msg::NoNcUpdate>
{
   // Keep references to the system state machine
   cycle_counter_t counter;
   pgm_t program;
   contact_cfg_t contact;
   contact_state_t state;

   quadrant_t current_selection, previous_selection;
   
public:
   explicit UIWorker() : counter {0}, program { pgm_t::PUSB }
   {
      current_selection = previous_selection = quadrant_t::none;
   }
   
   void on_receive(const fx::DispatcherStarted &msg)
   {
      draw_box(quadrant_t::counter);
      draw_counter();
   }

   void on_receive(const msg::Keypad &msg)
   {
      switch (msg.key_code)
      {
         case KEY_UP:
            previous_selection = current_selection--;
            draw_cursor(previous_selection, current_selection);
            break;
         case KEY_DOWN:
            previous_selection = current_selection++;
            draw_cursor(previous_selection, current_selection);
            break;
         case KEY_SELECT:
            ++counter;
            draw_counter();
            break;
         default:
            assert(0);
      }
      
   }
   
   void on_receive(const msg::NoNcUpdate &msg)
   {
      draw_nonc(msg.is_no);
   }

protected:

   void update_cycle_counter(cycle_counter_t newCount)
   {

   }
   
   void draw()
   {
      for ( uint8_t q=(*quadrant_t::none)+1; q<*quadrant_t::stop; ++q )
      {
         draw((quadrant_t)q);
      }
   }

   void draw(const quadrant_t qs)
   {
      draw_box(qs);
      
      switch(qs)
      {
         case quadrant_t::prog:     draw_prog();     break;
         case quadrant_t::activity: draw_activity(); break;
         case quadrant_t::counter:  draw_counter();  break;
         case quadrant_t::info:     draw_info();     break;
         default:
         assert(0);
      }
   }

   void draw_splash()
   {
      
   }
   
   void draw_prog()
   {

   }

   void draw_activity()
   {

   }

   void draw_counter()
   {
      static char counterString[6];
      snprintf(counterString, sizeof(counterString), "%.5u", this->counter);
      gfx_mono_draw_string(counterString, 15, 36, &sysfont);
   }
   
   void draw_nonc(bool isNo )
   {
      gfx_mono_draw_string(isNo ? "NO" : "NC", 34, 52, &sysfont);
   }

   void draw_info()
   {

   }
   
   void draw_box(const quadrant_t q)
   {
      gfx_mono_generic_draw_rect(0, 0, 48, 64, GFX_PIXEL_SET);
      gfx_mono_draw_horizontal_line(0, 15, 48, GFX_PIXEL_SET);
      gfx_mono_draw_horizontal_line(0, 31, 48, GFX_PIXEL_SET);
      gfx_mono_draw_horizontal_line(0, 47, 48, GFX_PIXEL_SET);
   }
   
   void draw_selection()
   {
      
   }
   
   void draw_cursor(quadrant_t from, quadrant_t to)
   {
      auto q2y = [](quadrant_t q) {
         return (*q) * 16;
      };
      
      if ( from != quadrant_t::none )
      {
         gfx_mono_draw_filled_rect(2, q2y(from), 8, 8, GFX_PIXEL_CLR);
      }
      
      if ( to != quadrant_t::none )
      {
         gfx_mono_put_bitmap(&cursor_bm, 2, q2y(to)+4);
         //gfx_mono_draw_filled_rect(2, q2y(to), 8, 8, GFX_PIXEL_SET);
      }
   }
   
};


#endif // ndef ui_worker_hpp__included
