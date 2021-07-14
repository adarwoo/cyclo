/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"
#include "keypad_tasklet.hpp"

#include "ui_router.hpp"
#include "console.hpp"
#include "sequencer_router.hpp"


int main(void)
{
   board_init();

   auto ui = UIRouter {};
   auto console = ConsoleWorker {};

   auto ui_bus = fx::Dispatcher<msg::packet, typestring_is("ui"), 256, 2>();

   auto root = fx::RootDispatcher<2>();
   
   root << ui_bus;
      ui_bus << ui << console;
   
   auto key_tasklet = KeypadTasklet {};

   rtos::start_scheduler();
}
