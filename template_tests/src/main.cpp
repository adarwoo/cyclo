/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"

#include "keypad_tasklet.hpp"
#include "nonc_tasklet.hpp"

#include "sequencer_worker.hpp"
#include "ui_worker.hpp"
#include "console_worker.hpp"


int main(void)
{
   // Initialise the board hardware (clocks, ios, buses etc.)
   board_init();
   
   // Create the workers and their dispatchers
   #if 0
   auto contact     = SequencerWorker {};
   auto rt_bus      = fx::Dispatcher<msg::sequencer_packet_t, typestring_is("sq"), 64>();

   auto console     = ConsoleWorker {};
   auto console_bus = fx::Dispatcher<msg::console_packet_t,   typestring_is("xt"), 64>();
#endif
   auto ui          = UIWorker();
   auto ui_bus      = fx::Dispatcher<msg::ui_packet_t,        typestring_is("ui"), 128, 1, 8>();
   
   auto root = fx::RootDispatcher</*Number of dispatchers=*/3>();

   //rt_bus << contact;
   ui_bus << ui;
   //console_bus << console;

   // Add by priority orders
   //root << rt_bus << console_bus << ui_bus;
   root << ui_bus;

   // Create the tasklets instance to pump key and contact events into fx
   auto key_tasklet = KeypadTasklet {};
   auto nonc_tasklet = NoNcTasklet {};

   // GO!
   rtos::start_scheduler();
}
