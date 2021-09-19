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
/*
 * The Cyclo main entry point.
 * There should be no static/global objects, so no contructor code is called
 * before main. Main is therefore trully the entry point.
 *
 * @author software@arreckx.com
 */
#include "asx.h"

#include "program_manager.hpp"
#include "sequencer_worker.hpp"
#include "ui_worker.hpp"
#include "keypad_tasklet.hpp"
#include "nonc_tasklet.hpp"
#include "console.hpp"

#include <fx.hpp>


int main( void )
{
   // Initialise the board hardware (clocks, IOs, buses etc.)
   board_init();

   // Create the 'programs' manager required throughout
   auto pgm_manager = ProgramManager{};

   // Create the workers and their dispatchers
   auto sequencer     = SequencerWorker{ pgm_manager };
   auto sequencer_bus = fx::Dispatcher<msg::packet_t, typestring_is( "sq" ), 32>();
   auto ui            = UIWorker{ pgm_manager };
   auto ui_bus        = fx::Dispatcher<msg::packet_t, typestring_is( "ui" ), 64, 1, 8>();

   ///< The root dispatcher (un-threaded) with 2 sub-dispatchers
   auto root = fx::RootDispatcher<2>();

   // Wire it all - add by priority order. The first added gets the messages first
   sequencer_bus << sequencer;
   ui_bus << ui;
   root << sequencer_bus << ui_bus;

   // Create the console task
   auto console = Console{ pgm_manager };

   // Create the tasklets instance to handle IRQ callbacks as events into fx
   auto key_tasklet  = KeypadTasklet{};
   auto nonc_tasklet = NoNcTasklet{ pgm_manager.get_contact() };

   // Do we need to auto-start a program?
   if ( pgm_manager.starts_automatically() )
   {
      fx::publish(msg::StartProgram{true});
   }

   // Start the scheduler - and go! The tasklets, tasks and workers are now loose
   rtos::start_scheduler();
}
