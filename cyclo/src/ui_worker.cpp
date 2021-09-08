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
 * The user interface worker
 *
 * @author guillaume.arreckx
 */
#include "ui_worker.hpp"

#include <logger.h>


using namespace rtos::tick;

namespace
{
   const char *const DOM = "ui_worker";
}

/** Create the timer for the splash */
UIWorker::SplashTimer::SplashTimer() : rtos::Timer<typestring_is( "tsplash" )>( 1.5_s )
{}

/** Publish a message on expiry */
void UIWorker::SplashTimer::run()
{
   LOG_HEADER( DOM );

   fx::publish( msg::EndOfSplash{} );
}

UIWorker::UIWorker( ProgramManager &program_manager )
   : model{ program_manager }
   , view{ model }
   , controller{ model, view }
   , program_manager{ program_manager }
{
   LOG_HEADER( DOM );
}

// @return true if the main screen can be updated with external changes
bool UIWorker::can_update()
{
   using namespace sml;

   bool is_in_splash = controller.is( "splash"_s );
   bool is_in_program_setup =
      controller.is<decltype( state<program_selection> )>( state<program_setup> );

   LOG_DEBUG(
      DOM, "%s",
      is_in_splash ? "X = is in splash" : is_in_program_setup ? "X = is in pgm setup" : "OK" );

   return not ( is_in_splash or is_in_program_setup );
}

// --------------------------------------------------------------
// Message handlers
// --------------------------------------------------------------
void UIWorker::on_receive( const fx::DispatcherStarted &msg )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "DispatcherStarted" );

   splash_timer.start();
}

void UIWorker::on_receive( const msg::EndOfSplash &msg )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "EndOfSplash" );

   // Feed into the SM
   process_event( controller, splash_timeout{} );

   // Is there an auto_start program?
   if ( program_manager.starts_automatically() )
   {
      // Load it (unless it's 0)
      uint8_t selected = program_manager.get_selected();

      if ( selected > 0 )
      {
         // Parse it
         program_manager.load( selected );
      }

      // Start!
      fx::publish( msg::StartProgram{ true } );

      // Set the running state
      model.set_state( UIModel::program_state_t::running );
   }
}

void UIWorker::on_receive( const msg::Keypad &msg )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "Keypad" );

   switch ( msg.key_code )
   {
   case KEY_UP: process_event( controller, up{} ); break;
   case KEY_DOWN: process_event( controller, down{} ); break;
   case KEY_SELECT: process_event( controller, push{} ); break;
   default: assert( 0 );
   }
}

void UIWorker::on_receive( const msg::NoNcUpdate &msg )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "NoNcUpdate" );

   if ( can_update() )
   {
      view.draw_nonc();
   }
}

void UIWorker::on_receive( const msg::CounterUpdate & )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "CounterUpdate" );

   if ( can_update() )
   {
      view.draw_counter();
   }
}

void UIWorker::on_receive( const msg::ContactUpdate & )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "ContactUpdate" );

   if ( can_update() )
   {
      view.draw_contact();
   }
}

void UIWorker::on_receive( const msg::USBConnected & )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "USBConnected" );

   model.set_state(UIModel::program_state_t::usb);
   process_event( controller, usb_on{} );
}

void UIWorker::on_receive( const msg::USBDisconnected & )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "USBDisconnected" );

   model.set_state(UIModel::program_state_t::stopped);
   process_event( controller, usb_off{} );
}

void UIWorker::on_receive( const msg::ProgramIsStopped & )
{
   LOG_HEADER( DOM );
   LOG_TRACE( DOM, "ProgramIsStopped" );

   process_event( controller, pgm_stopped{} );
}
