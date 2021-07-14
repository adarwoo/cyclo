#ifndef console_router_hpp__included
#define console_router_hpp__included
/*
 * UI Woker
 */
#include "fx.hpp"
#include "msg_defs.hpp"

using namespace rtos::tick;

class ConsoleRouter : public etl::message_router<TerminalWorker, msg::NoNcUpdate, msg::Keypad>
{
public:
   ConsoleRouter(etl::message_router_id_t id_) : etl::message_router<ConsoleRouter, msg::NoNcUpdate, msg::Keypad>(id_) {}

   void on_receive(const msg::NoNcUpdate &msg)
   {
       
   }

   void on_receive(const msg::Keypad &msg)
   {
      auto log = Trace(TRACE_IDLE);
      rtos::delay(50_ms);
   }

   void on_receive_unknown(const etl::imessage &msg) {}
};

#endif // ndef console_router_hpp__included
