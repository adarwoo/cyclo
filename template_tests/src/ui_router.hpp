#ifndef ui_router_hpp__included
#define ui_router_hpp__included
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

using namespace rtos::tick;

enum class quadrant_t : uint8_t
{
   none,
   prog = 1,
   activity,
   counter,
   info,
   stop
};

constexpr uint8_t operator*(const quadrant_t q)
   { return static_cast<uint8_t>(q); }


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
   no,
   nc
};

enum class contact_state_t : uint8_t
{
   opened,
   closed
};

class HMI
{
   quadrant_t current_selection;

   // Keep references to the system state machine
   cycle_counter_t &counter;
   pgm_t &program;
   contact_cfg_t &contact;
   contact_state_t &state;

public:
   HMI(cycle_counter_t &cnt, pgm_t &pgm, contact_cfg_t &cfg, contact_state_t &state) :
      current_selection(quadrant_t::none), counter(cnt), program(pgm), contact(cfg), state(state)
   {
      draw_splash();
   }

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

   }

   void draw_info()
   {

   }
   
   void draw_box(const quadrant_t q)
   {
      gfx_coord_t y1 {(*q-1) * 16};
      gfx_coord_t y2 {y1 + 16};

      gfx_mono_generic_draw_rect(0, 0, 47, 16, GFX_PIXEL_SET);
   }
};

   // Keep references to the system state machine
   cycle_counter_t counter;
   pgm_t program;
   contact_cfg_t contact;
   contact_state_t state;
   
HMI hmi {counter, program, contact, state};

// On UI timer
//
struct UIRouter : public fx::Worker<UIRouter, fx::DispatcherStarted, msg::RefreshUI, msg::Keypad>
{
   void on_receive(const fx::DispatcherStarted &msg)
   {
      hmi.draw_box(quadrant_t::counter);
   }

   void on_receive(const msg::RefreshUI &msg)
   {
   }

   void on_receive(const msg::Keypad &msg)
   {
      auto log = Trace(TRACE_ERR);
      rtos::delay(50_ms);
   }
};


#endif // ndef ui_router_hpp__included
