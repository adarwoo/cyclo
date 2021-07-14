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

   auto root = fx::RootDispatcher<2>();
   auto ui_bus = fx::Dispatcher<msg::packet, typestring_is("ui"), 256, 2>();
   auto ui = UIRouter {};
   
   root.subscribe(ui_bus);
   ui_bus.subscribe(ui);

//   auto seq = SequencerRouter(msg::router::SEQUENCER);
   //auto seq_bus = fx::Dispatcher<msg::packet, typestring_is("seq"), 255, 1>();
   //d2.subscribe(seq);
   
   #if 0
   using root_t = fx::root_dispatcher<2, msg::packet>;
   
   auto root = root_t() 
      << root_t::dispatcher<typestring_is("ui"), 255, 1>() 
         << UIRouter()
         << Console()
      << root_t::dispatcher<typestring_is("seq"), 255, 1>() 
         << SequencerRouter();
   #endif

   auto key_tasklet = KeypadTasklet {};

   rtos::start_scheduler();
}
