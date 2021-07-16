#ifndef sequencer_router_hpp__included
#define sequencer_router_hpp__included
/*
 * Sequencer router
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"
#include "msg_defs.hpp"

using namespace rtos::tick;

class SequencerRouter : public etl::message_router<SequencerRouter, msg::NoNcUpdate>
{
public:
   SequencerRouter(etl::message_router_id_t id_) : etl::message_router<SequencerRouter, msg::NoNcUpdate>(id_) {}

   void on_receive(const msg::NoNcUpdate &msg)
   {
      auto log = Trace(TRACE_TICK);
      rtos::delay(100_ms);
   }

   void on_receive_unknown(const etl::imessage &msg) {}
};


#endif // ndef sequencer_router_hpp__included