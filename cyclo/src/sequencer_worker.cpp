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

   // Simple race prevention : invalidate the current timer instance
   //  to prevent a race where the timer message is already in the queue
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

      // If a delay exists
      if ( cmd->delay_ms )
      {
         // Fire a new timers
         timer.set_param( ++timer_counter );
         timer.set_period( rtos::tick::from_ms( cmd->delay_ms ) );
         timer.start();
      }
   }
   else
   {
      // The program has stopped - let the GUI know
      fx::publish( msg::ProgramIsStopped{} );
   }
}
