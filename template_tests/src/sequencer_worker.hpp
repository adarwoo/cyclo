#ifndef sequencer_worker_hpp_included
#define sequencer_worker_hpp_included
/*
 * Sequencer router
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "fx.hpp"
#include "msg_defs.hpp"

using namespace rtos::tick;

class SequencerWorker : public fx::Worker<
   SequencerWorker,
   msg::NoNcUpdate>
{
public:
   void on_receive(const msg::NoNcUpdate &msg)
   {
      rtos::delay(100_ms);
   }

   void on_receive_unknown(const etl::imessage &msg) {}
};


#endif // ndef sequencer_worker_hpp_included