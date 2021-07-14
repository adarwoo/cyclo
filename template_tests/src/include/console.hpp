#ifndef console_workerer_hpp__included
#define console_workerer_hpp__included
/*
 * UI Woker
 */
#include "fx.hpp"
#include "msg_defs.hpp"

using namespace rtos::tick;

class ConsoleWorker : public fx::Worker<ConsoleWorker, msg::NoNcUpdate, msg::Keypad>
{
public:
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

#endif // ndef console_workerer_hpp__included
