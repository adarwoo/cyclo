#ifndef sequencer_worker_hpp_included
#define sequencer_worker_hpp_included
/*
 * Sequencer router
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include <fx.hpp>

#include "cyclo_manager.hpp"
#include "msg_defs.hpp"


using namespace rtos::tick;


class SequencerWorker
   : public fx::Worker<SequencerWorker, msg::StartProgram, msg::StopProgram, msg::SequenceNext>
{
   ///< Timer in between sequences
   class SeqTimer : public rtos::Timer<typestring_is( "tseq" ), uint32_t>
   {
   protected:
      virtual void run() override { fx::publish( msg::SequenceNext{} ); }
   } timer;

   ///< Prevent race - associate a count to each timer run
   uint32_t timer_counter;

   ///< Store the time left when resuming from pause
   rtos::tick_t ticks_left;

   ///< Access to the command manager
   CycloManager &pgm_man;

public:
   SequencerWorker( CycloManager &pgm_man );

   // Activate the sequencer. This resets the program
   void on_receive( const msg::StartProgram &msg );

   void on_receive( const msg::StopProgram &msg );

   void on_receive( const msg::SequenceNext &msg );

protected:
   void execute_next();
};


#endif  // ndef sequencer_worker_hpp_included