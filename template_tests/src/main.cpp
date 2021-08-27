/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include <fx.hpp>

#include "asx.h"

#include "cyclo_manager.hpp"

#include "keypad_tasklet.hpp"
#include "nonc_tasklet.hpp"

#include "sequencer_worker.hpp"
#include "ui_worker.hpp"
#include "console_worker.hpp"


int main(void)
{
   // Initialise the board hardware (clocks, IOs, buses etc.)
   board_init();
   
   // Create the manager required for all others
   auto manager = CycloManager{};
   
   // Create the workers and their dispatchers
   auto sequencer     = SequencerWorker{manager};
   auto sequencer_bus = fx::Dispatcher<msg::packet_t, typestring_is("sq"), 64>();

   auto ui            = UIWorker{manager};
   auto ui_bus        = fx::Dispatcher<msg::packet_t, typestring_is("ui"), 64, 1, 8>();

#if 0
   auto console       = ConsoleWorker{manager};
   auto console_bus   = fx::Dispatcher<msg::packet_t, typestring_is("xt"), 64>();
#endif
   
   ///< The root dispatcher (un-threaded) with 3 sub-dispatchers
   auto root = fx::RootDispatcher<3>();

   sequencer_bus << sequencer;
   ui_bus << ui;

   // Add by priority orders
   root << sequencer_bus << ui_bus;

   // Create the tasklets instance to pump key and contact events into fx
   auto key_tasklet = KeypadTasklet{};
   auto nonc_tasklet = NoNcTasklet{manager.get_contact()};

   // GO!
   rtos::start_scheduler();
}
