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

#include "cyclo_manager.hpp"


using namespace rtos::tick;


class SequencerWorker : public fx::Worker<SequencerWorker,
   msg::StartProgram,
   msg::StopProgram,
   msg::SequenceNext
   >
{
   ///< Timer in between sequences
   class SeqTimer : public rtos::Timer<typestring_is("tseq"), uint32_t>
   {
   protected:
      virtual void run() override 
      { 
         fx::publish(msg::SequenceNext {}); 
      }
   } timer;
   
   ///< Prevent race - associate a count to each timer run
   uint32_t timer_counter;
   
   ///< Store the time left when resuming from pause
   rtos::tick_t ticks_left;
   
   ///< Access to the command manager
   CycloManager &cm;
   
   
public:
   SequencerWorker(CycloManager &cm) : timer_counter{100}, ticks_left{0}, cm{cm} {}

   // Activate the sequencer. This resets the program
   void on_receive(const msg::StartProgram &msg)
   {
      if ( msg.from_start ) {
         // Reset the counter
         cm.set_counter(0);
      
         // Update the GUI
         fx::publish(msg::CounterUpdate{});

         // Start from the start
         cm.get_command().start();
         
         ticks_left = 0;
      } 
      else if ( ticks_left )
      {
         timer.set_param(timer_counter);
         timer.set_period(ticks_left);
         timer.start();
         
         return;
      }         
         
      execute_next();
   }     

   void on_receive(const msg::StopProgram &msg)
   {
      ticks_left = timer.get_time_left_until_expiry();
      
      // Invalidate the current timer instance to prevent a race (the timer message is already in the queue)
      ++timer_counter;

      // Stop the timer
      timer.stop();
   }

   void on_receive(const msg::SequenceNext &msg)
   {
      // Check the timer is still valid (avoid race)
      if ( timer.get_param() == timer_counter ) {
         execute_next();
      }
   }      
protected:
   void execute_next() {
      // Pick the first command and apply it
      auto &command = cm.get_command();
      
      // Grab the first item and move the iterator
      const CommandItem *item = command.iterate();
      
      // Make sure not the last
      if ( item != nullptr ) {
         // Execute the item
         switch ( item->command ) {
         case CommandItem::close:
            cm.get_contact().set(Contact::close); 
            break;
         case CommandItem::open:
            cm.get_contact().set(Contact::open);
            break;
         case CommandItem::delay:
            // Do nothing
            break;
         case CommandItem::loop:
            command.start();

            cm.set_counter(cm.get_counter() + 1);
            fx::publish(msg::CounterUpdate{});
               
            // Avoid recursion - save the stack, save the trouble - post again
            fx::publish(msg::SequenceNext{});
            return;
         }

         // Fire a new timers
         timer.set_param(++timer_counter);
         timer.set_period(rtos::tick::from_ms(item->delay_ms));
         timer.start();
      }
   }
};


#endif // ndef sequencer_worker_hpp_included