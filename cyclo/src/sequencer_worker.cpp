/*
 * Sequencer router
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */

#include "sequencer_worker.hpp"

#include <logger.h>


using namespace rtos::tick;

namespace
{
   const char *const DOM = "sq.worker";
}


SequencerWorker::SequencerWorker( ProgramManager &pgm_man )
   : timer_counter{ 100 }, ticks_left{ 0 }, pgm_man{ pgm_man }
{}

// Activate the sequencer. This resets the program
void SequencerWorker::on_receive( const msg::StartProgram &msg )
{
   LOG_TRACE( DOM, "StartProgram" );

   if ( msg.from_start )
   {
      // Reset the counter
      pgm_man.set_counter( 0 );

      // Update the GUI
      fx::publish( msg::CounterUpdate{} );

      // Start from the start
      pgm_man.get_active_program().start();

      ticks_left = 0;
   }
   else if ( ticks_left )
   {
      timer.set_param( timer_counter );
      timer.set_period( ticks_left );
      timer.start();

      return;
   }

   execute_next();
}

void SequencerWorker::on_receive( const msg::StopProgram &msg )
{
   LOG_TRACE( DOM, "StopProgram" );

   ticks_left = timer.get_time_left_until_expiry();

   // Invalidate the current timer instance to prevent a race (the timer message is already in
   // the queue)
   ++timer_counter;

   // Stop the timer
   timer.stop();
}

void SequencerWorker::on_receive( const msg::SequenceNext &msg )
{
   LOG_TRACE( DOM, "SequenceNext" );

   // Check the timer is still valid (avoid race)
   if ( timer.get_param() == timer_counter )
   {
      execute_next();
   }
}

void SequencerWorker::execute_next()
{
   LOG_HEADER( DOM );

   // Pick the first command and apply it
   auto &pgm = pgm_man.get_active_program();

   // Grab the first item and move the iterator
   const Command *cmd = pgm.iterate();

   // Make sure not the last
   if ( cmd != nullptr )
   {
      // Execute the item
      switch ( cmd->command )
      {
      case Command::close: pgm_man.get_contact().set( Contact::close ); break;
      case Command::open: pgm_man.get_contact().set( Contact::open ); break;
      case Command::delay:
         // Do nothing
         break;
      case Command::loop:
         // Start all over
         pgm_man.get_active_program().start();

         pgm_man.set_counter( pgm_man.get_counter() + 1 );
         fx::publish( msg::CounterUpdate{} );

         // Avoid recursion - save the stack, save the trouble - post again
         fx::publish( msg::SequenceNext{} );
         return;
      }

      // Fire a new timers
      timer.set_param( ++timer_counter );
      timer.set_period( rtos::tick::from_ms( cmd->delay_ms ) );
      timer.start();
   }
}
