#ifndef console_router_hpp__included
#define console_router_hpp__included
/*
 * UI Woker
 */
#include "fx.hpp"
#include "msg_defs.hpp"

using namespace rtos::tick;

class UIWorker : public etl::message_router<UIWorker, msg::RefreshUI, msg::Keypad>
{
public:
   UIWorker(etl::message_router_id_t id_) : etl::message_router<UIWorker, msg::RefreshUI, msg::Keypad>(id_) {}

   void on_receive(const msg::RefreshUI &msg)
   {
      
   }

   void on_receive(const msg::Keypad &msg)
   {
      auto log = Trace(TRACE_ERR);
      rtos::delay(50_ms);
   }

   void on_receive_unknown(const etl::imessage &msg) {}
};

#endif // ndef console_router_hpp__included
