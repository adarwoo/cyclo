/*
 * ui_worker.cpp
 *
 * Created: 24/08/2021 00:49:30
 *  Author: micro
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

UIWorker::UIWorker( CycloManager &cm )
   : model{ cm }, view{ model }, controller{ model, view }, cm{ cm }
{
   LOG_HEADER( DOM );
}

// @return true if the main screen can be updated with external changes
bool UIWorker::can_update()
{
   using namespace sml;

   return not controller.is<decltype( state<program_selection> )>( state<program_setup> )
          and not controller.is( "splash"_s );
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
   if ( cm.starts_automatically() )
   {
      // Load it (unless it's 0)
      uint8_t selected = cm.get_selected();

      if ( selected > 0 )
      {
         // Parse it
         cm.load( selected );
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
