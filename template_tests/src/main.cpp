/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"
#include "keypad_tasklet.hpp"

#include "ui_router.hpp"
#include "console_router.hpp"
#include "sequencer_router.hpp"


int main(void)
{
   board_init();

   //auto root = fx::RootDispatcher<2>();
   
   auto ui = UIRouter();
//   auto seq = SequencerRouter(msg::router::SEQUENCER);

   auto ui_bus = fx::Dispatcher<msg::packet, typestring_is("ui"), 1024, 2>();
   ui_bus.subscribe(ui);

//   auto d2 = fx::Dispatcher<msg::packet, typestring_is("seq"), 255, 1>();
//   d2.subscribe(seq);

   KeypadTasklet key_task(ui_bus);

   rtos::start_scheduler();
}
