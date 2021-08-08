#ifndef ui_worker_hpp__included
#define ui_worker_hpp__included
/*
 * UI Worker
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "asf.h"

#include "fx.hpp"
#include "msg_defs.hpp"

#include "etl/limits.h"
#include "etl/optional.h"

#include "ui_model.hpp"
#include "ui_view.hpp"
#include "ui_controller.hpp"

using namespace rtos::tick;

class UIWorker : public fx::Worker<
   UIWorker,
   fx::DispatcherStarted, 
   msg::EndOfSplash,
   msg::Keypad,
   msg::NoNcUpdate,
   msg::ContactUpdate
>
{
   using UIController = sml::sm<sm_cyclo>;
   
   // Create a timer
   class Splash_timer : public rtos::Timer<typestring_is("tsplash")>
   {
   public:
      Splash_timer() : rtos::Timer<typestring_is("tsplash")>(1.5_s) {}
   protected:   
      virtual void run() override
      {
         fx::publish(msg::EndOfSplash {});
      }
   } splash_timer;
   
   UIModel model;
   UIView  view;
   UIController controller;
   
public:
   explicit UIWorker() : model{}, view{model}, controller{model, view}
   {
   }
   
   // @return true if the main screen can be updated with external changes
   bool can_update()
   {
      using namespace sml;
      
      return 
         not controller.is<decltype(state<program_selection>)>(state<program_setup>)
         and
         not controller.is("splash"_s);
   }
   
   // --------------------------------------------------------------
   // Message handlers
   // --------------------------------------------------------------
   void on_receive(const fx::DispatcherStarted &msg)
   {
      splash_timer.start();
   }

   void on_receive(const msg::EndOfSplash &msg)
   {
      process_event(controller, splash_timeout{});
   }

   void on_receive(const msg::Keypad &msg)
   {
      switch (msg.key_code)
      {
        case KEY_UP: process_event(controller, up {}); break;
        case KEY_DOWN: process_event(controller, down{}); break;
        case KEY_SELECT: process_event(controller, push{}); break;
        default:
           assert(0);
      }
   }
   
   void on_receive(const msg::NoNcUpdate &msg)
   {
      if ( can_update() )
      {
         view.draw_nonc();
      }
   }
   
   void on_receive(const msg::CounterUpdate &)
   {
      if ( can_update() )
      {
         view.draw_counter();
      }         
   }

   void on_receive(const msg::ContactUpdate &)
   {
      if ( can_update() )
      {
         view.draw_contact();
      }
   }
};


#endif // ndef ui_worker_hpp__included
