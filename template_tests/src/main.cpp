/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"

#include "keypad_tasklet.hpp"
#include "nonc_tasklet.hpp"

#include "ui_worker.hpp"
#include "sequencer_worker.hpp"
#include "console_worker.hpp"


int main(void)
{
   board_init();

   auto ui = UIWorker();
   auto console = ConsoleWorker {};

   auto ui_bus = fx::Dispatcher<msg::packet, typestring_is("ui"), 256, 2>();

   auto root = fx::RootDispatcher<2>();

   root << ui_bus;

   ui_bus << ui << console;

   auto key_tasklet = KeypadTasklet {};
   auto nonc_tasklet = NoNcTasklet {};

   rtos::start_scheduler();
}
