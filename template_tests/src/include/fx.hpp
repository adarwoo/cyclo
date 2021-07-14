#ifndef FX_H_
#define FX_H_
/*
 * fx.h
 * A bus which differ through a queue and a task
 *
 * Created: 08/07/2021 19:04:29
 *  Author: micro
 */
#include "etl/message_bus.h"
#include "rtos.hpp"

namespace fx
{
   /** Cannot be bound to the root dispatcher since Td */
   inline static etl::imessage_bus *root_dispatcher = nullptr;

   inline static etl::message_router_id_t current_worker_id = 0;

   
   struct DispatcherStartedMessage : etl::message<255>
   {
   };   
   
   template<class D, class... T>
   struct Worker : public etl::message_router<D, T...>
   {
      Worker() : etl::message_router<D, T...>(++current_worker_id)
      {}

      virtual void on_dispatcher_started()
      {}

      virtual void on_receive(const DispatcherStartedMessage &)
      {
         on_dispatcher_started();
      }

      virtual void on_receive_unknown(const etl::imessage &msg)
      {}
   };


   template <class TMsgPacket, class TName, const size_t TStackSize, uint_least8_t MAX_ROUTERS_=1>
   class Dispatcher : public etl::imessage_bus, public rtos::Task<TName, TStackSize>
   {
      rtos::Queue<TMsgPacket, MAX_ROUTERS_> queue;

   public:
      Dispatcher() : etl::imessage_bus(router_list), queue()
      {
         //assert(root_dispatcher != nullptr);
         //root_dispatcher->subscribe(*this);
         
         this->run();
      }

      virtual void receive(const etl::imessage &message) override
      {
         auto packet = TMsgPacket(message);
         queue.send(packet);
      }

      /** Dispatcher's task entry point */
      void default_handler() override
      {
         TMsgPacket *packet = nullptr;
         
         //for ( auto *worker : router_list ) 
         //{
         //   send_message(*worker, DispatcherStartedMessage {});
         //}

         while (true)
         {
            // Get the shared message from the queue.
            queue.receive(packet);

            // Send it to the base implementation for routing.
            receive(packet->get());
         }
      }

  private:
      etl::vector<etl::imessage_router*, MAX_ROUTERS_> router_list;  
   };


   template<const size_t TSize>
   class RootDispatcher : etl::message_bus<TSize>
   {
   public:
      RootDispatcher() : etl::message_bus<TSize>()
      {
         assert(root_dispatcher != nullptr);
         root_dispatcher = this;
      }
   };

} // namespace fx

#endif /* FX_H_ */