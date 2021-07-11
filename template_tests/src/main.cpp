/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "rtos.hpp"
#include "trace.h"
#include "keypad.h"
#include "ui.hpp"

#include "timers.h"

#include "fx.hpp"

#include "msg_defs.hpp"

#include "etl/shared_message.h"

using namespace rtos::tick;


class UIWorker : public etl::message_router<UIWorker, msg::RefreshUI, msg::Keypad>
{
public:
   UIWorker(etl::message_router_id_t id_) : etl::message_router<UIWorker, msg::RefreshUI, msg::Keypad>(id_) {}
   
   void on_receive(const msg::RefreshUI& msg)   {}      
   void on_receive(const msg::Keypad& msg)
   {
      auto log = Trace(TRACE_ERR);
      rtos::delay(50_ms);
   }
      
   void on_receive_unknown(const etl::imessage& msg) {}      
};


class TerminalWorker : public etl::message_router<TerminalWorker, msg::NoNcUpdate, msg::Keypad>
{
public:
   TerminalWorker(etl::message_router_id_t id_) : etl::message_router<TerminalWorker, msg::NoNcUpdate, msg::Keypad>(id_) {}

   void on_receive(const msg::NoNcUpdate& msg)   {}
   
   void on_receive(const msg::Keypad& msg)
   {
      auto log = Trace(TRACE_IDLE);
      rtos::delay(50_ms);
   }
   
   void on_receive_unknown(const etl::imessage& msg) {}            
};


class SequencerWorker : public etl::message_router<SequencerWorker, msg::NoNcUpdate>
{
public:
   SequencerWorker(etl::message_router_id_t id_) : etl::message_router<SequencerWorker, msg::NoNcUpdate>(id_) {}

   void on_receive(const msg::NoNcUpdate& msg) 
   {
      auto log = Trace(TRACE_TICK);
      rtos::delay(100_ms);
   }
   
   void on_receive_unknown(const etl::imessage& msg) {}
};


template<class TName, const size_t TStackSize, const size_t N>
class TaskBus : public etl::message_bus<N>, public rtos::Task<TName, TStackSize>
{
   rtos::Queue<msg::packet, N> queue;
   using base_t = etl::message_bus<N>;
   
public:
   TaskBus() : etl::message_bus<N>(), queue()
   {
      this->run();
   }

   virtual void receive(const etl::imessage& message) override
   {
      auto packet = msg::packet(message);
      queue.send(packet);
   }      
  
   void default_handler() override
   {
      msg::packet *packet = nullptr;
    
      while (true)
      {
         // Get the shared message from the queue.
         queue.receive(packet);

         // Send it to the base implementation for routing.
         base_t::receive(packet->get());
      }     
   }
};


struct FlashLed : public rtos::Timer<typestring_is("t1")>
{
   FlashLed() : rtos::Timer<typestring_is("t1")>(50_ms, false) {}

   void blink()      
   {
      trace_set(TRACE_ERR);
      this->start();
   }
   
   virtual void run() override
   {
      trace_clear(TRACE_ERR);
   }
};


class KeypadTasklet : public rtos::Tasklet
{
   etl::imessage_bus &root_dispatcher; 
   
public:
   KeypadTasklet(etl::imessage_bus &root) : root_dispatcher(root)
   {
      keypad_register_callback(
         KEY_UP | KEY_DOWN | KEY_SELECT, 
         callback_from_isr, 
         this
      );
   }
   
   static void callback_from_isr(uint8_t k, void *param)
   {
      KeypadTasklet *this_ = (KeypadTasklet*)param;
      this_->schedule_from_isr((uint32_t)k);
   }
   
   virtual void run(uint32_t key) override
   {
      msg::Keypad msg;
      msg.key_code = key;
      
      root_dispatcher.receive(msg);
      root_dispatcher.receive(msg::NoNcUpdate{});
   }
};

int main(void)
{
   auto ui = UIWorker(msg::router::UI);
   auto term = TerminalWorker(msg::router::TERMINAL);
   auto seq = SequencerWorker(msg::router::SEQUENCER);

   auto d1 = TaskBus<typestring_is("d1"), 255, 2>();
   d1.subscribe(ui);
   d1.subscribe(term);
   
   auto d2 = TaskBus<typestring_is("d2"), 255, 1>();
   d2.subscribe(seq);
   
   auto root = etl::message_bus<2>();
   root.subscribe(d1);
   root.subscribe(d2);

   board_init();
   keypad_init();
   KeypadTasklet key_task(root);

   rtos::start_scheduler();
}
