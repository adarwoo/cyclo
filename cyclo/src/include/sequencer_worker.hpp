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
#ifndef sequencer_worker_hpp_included
#define sequencer_worker_hpp_included
/*
 * Sequencer router
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include <fx.hpp>

#include "msg_defs.hpp"
#include "program_manager.hpp"


using namespace rtos::tick;


class SequencerWorker
   : public fx::Worker<
        SequencerWorker,
        msg::CheckHealth,
        msg::StartProgram,
        msg::StopProgram,
        msg::SequenceNext>
{
   ///< Timer in between sequences
   rtos::Timer<typestring_is( "tseq" ), uint32_t> timer;

   ///< Prevent race - associate a count to each timer run
   uint32_t timer_counter;

   ///< Store the time left when resuming from pause
   rtos::tick_t ticks_left;

   ///< Local copy of the program to avoid race
   Program program;

   ///< Access to the command manager
   ProgramManager &pgm_man;

public:
   SequencerWorker( ProgramManager &pgm_man );

   void on_receive( const msg::CheckHealth &msg );

   // Activate the sequencer. This resets the program
   void on_receive( const msg::StartProgram &msg );

   void on_receive( const msg::StopProgram &msg );

   void on_receive( const msg::SequenceNext &msg );


protected:
   void execute_next();
};


#endif  // ndef sequencer_worker_hpp_included