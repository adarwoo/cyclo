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
        msg::USBConnected,
        msg::USBDisconnected,
        msg::ProgramIsStopped>
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
   void on_receive( const msg::USBDisconnected& );
   void on_receive( const msg::ProgramIsStopped& );
};


#endif  // ndef ui_worker_hpp__included
