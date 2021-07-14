#ifndef FX_H_
#define FX_H_
/*
 * fx.h
 * Framework for connecting consumer/producers statically
 *
 * Created: 08/07/2021 19:04:29
 *  Author: micro
 */


#include "etl/message.h"

#include "typestring.hpp"

#include "etl/message_router.h"
#include "etl/message_bus.h"
#include "etl/function.h"
#include "etl/delegate.h"

#include "msg_defs.hpp"
#include "trace.h"
#include "rtos.hpp"


namespace fx
{
   static inline etl::imessage_bus *root_dispatcher = nullptr;
   
   template<const size_t MAXBUSES>
   class RootDispatcher : public etl::message_bus<MAXBUSES>
   {
   public:
      RootDispatcher()
      {
         assert(root_dispatcher == nullptr);
         root_dispatcher = this;
      }
   };
   
   void publish(const etl::imessage& msg_)
   {
      assert(root_dispatcher);
      root_dispatcher->receive(msg_);
   }
   
   inline static etl::message_router_id_t message_router_auto_id {0};

   constexpr etl::message_id_t DISPATCHER_STARTED = 255;
   
   struct DispatcherStarted : etl::message<DISPATCHER_STARTED> {};
      
   template<class T, class... TMsgs>
   struct Worker : public etl::message_router<T, TMsgs...>
   {
      Worker<T, TMsgs...>() : etl::message_router<T, TMsgs...>(++message_router_auto_id) {}
      void on_receive_unknown(const etl::imessage& msg) {}
      void on_start();
   };
   
   template <class TPacket, class TName, const size_t STACKSIZE, const uint_least8_t MAX_ROUTERS, const size_t QUEUESIZE=4>
   class Dispatcher : public etl::message_bus<MAX_ROUTERS>, public rtos::Task<TName, STACKSIZE>
   {
      rtos::Queue<TPacket, QUEUESIZE> queue;
         
   public:
      Dispatcher() : etl::message_bus<MAX_ROUTERS>()
      {
         this->run();
      }
      
      template<class T, class... TMsg> Dispatcher &operator<<(Worker<T, TMsg...>& w)
      {
         this->subscribe(w);
         return *this;
      }

      template<class T, class... TMsg> Dispatcher &operator<<(Worker<T, TMsg...>&& w)
      {
         Worker<T, TMsg...> *p = (Worker<T, TMsg...> *)malloc(sizeof(Worker<T, TMsg...>));
         assert(p != nullptr);
         ::new (p) Worker<T, TMsg...>();
         
         this->subscribe(*p);
         return *this;
      }
      
   protected:
      void receive(const etl::imessage& msg_) override
      {
         auto packet = TPacket(msg_);
         queue.send(packet);
      }
      
      void default_handler()
      {
         // Thread is started. Let all worker know ti
         etl::message_bus<MAX_ROUTERS>::receive(etl::imessage_router::ALL_MESSAGE_ROUTERS, DispatcherStarted {});
         
         while (true)
         {
            auto packet = TPacket();

            queue.receive(packet);
            auto &msg = packet.get();
            etl::message_bus<MAX_ROUTERS>::receive(etl::imessage_router::ALL_MESSAGE_ROUTERS, msg);
         }
      }
   };
}


#endif /* FX_H_ */