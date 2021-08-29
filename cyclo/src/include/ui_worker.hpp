#ifndef ui_worker_hpp__included
#define ui_worker_hpp__included
/*
 * UI Worker
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include <fx.hpp>

#include <etl/limits.h>
#include <etl/optional.h>

#include "program_manager.hpp"
#include "msg_defs.hpp"
#include "ui_controller.hpp"
#include "ui_model.hpp"
#include "ui_view.hpp"


class UIWorker
   : public fx::Worker<
        UIWorker,

        fx::DispatcherStarted,

        msg::EndOfSplash,
        msg::Keypad,
        msg::NoNcUpdate,
        msg::ContactUpdate,
        msg::CounterUpdate,
        msg::USBConnected>
{
   using UIController = sml::sm<sm_cyclo>;

   // Create a timer
   struct SplashTimer : public rtos::Timer<typestring_is( "tsplash" )>
   {
      SplashTimer();

   protected:
      virtual void run() override;
   };

   SplashTimer splash_timer;

   // MVC instances. The model is only a facade
   UIModel      model;
   UIView       view;
   UIController controller;

   // Access to the manager
   ProgramManager &program_manager;

public:
   explicit UIWorker( ProgramManager &program_manager );

   // @return true if the main screen can be updated with external changes
   bool can_update();

   // --------------------------------------------------------------
   // Message handlers
   // --------------------------------------------------------------
   void on_receive( const fx::DispatcherStarted &msg );
   void on_receive( const msg::EndOfSplash &msg );
   void on_receive( const msg::Keypad &msg );
   void on_receive( const msg::NoNcUpdate &msg );
   void on_receive( const msg::CounterUpdate & );
   void on_receive( const msg::ContactUpdate & );
   void on_receive( const msg::USBConnected& );
   
};


#endif  // ndef ui_worker_hpp__included
