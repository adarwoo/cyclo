#ifndef ui_router_hpp__included
#define ui_router_hpp__included
/*
 * UI Woker
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"
#include "msg_defs.hpp"
#include "etl/limits.h"

using namespace rtos::tick;

enum class quadrant_t : uint8_t
{
   none,
   prog = 1 << 1,
   activity = 1 << 2,
   counter = 1 << 3,
   info = 1 << 4,
   //all = 255
};

using quadrants_t = quadrant_t;

bool operator&(quadrants_t qset, quadrant_t qval)
{
   uint8_t u8_set = static_cast<uint8_t>(qset);
   uint8_t u8_v = static_cast<uint8_t>(qval);
   
   return (u8_set & u8_v);
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
   quadrant_t selection;

   // Keep references to the system state machine
   cycle_counter_t &counter;
   pgm_t &program;
   contact_cfg_t &contact;
   contact_state_t &state;

public:
   HMI(cycle_counter_t &cnt, pgm_t &pgm, contact_cfg_t &cfg, contact_state_t &state) :
      selection(quadrant_t::none), counter(cnt), program(pgm), contact(cfg), state(state)
   {
      draw_splash();
   }

   void update_cycle_counter(cycle_counter_t newCount)
   {

   }

   void draw(const quadrants_t qs)
   {
      if ( qs & quadrant_t::prog )     { draw_prog(); }
      if ( qs & quadrant_t::activity ) { draw_activity(); }
      if ( qs & quadrant_t::counter )  { draw_counter(); }
      if ( qs & quadrant_t::info )     { draw_info(); }
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
};

// On UI timer
//
struct UIRouter : public fx::Worker<UIRouter, msg::RefreshUI, msg::Keypad>
{
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
