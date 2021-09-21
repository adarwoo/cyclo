#ifndef fx_hpp_was_included
#define fx_hpp_was_included
/*
 * fx.h
 * Framework for connecting consumer/producers statically
 *
 * Created: 08/07/2021 19:04:29
 *  Author: micro
 */

#include <rtos.hpp>
#include <typestring.hpp>

#include <etl/delegate.h>
#include <etl/function.h>
#include <etl/message.h>
#include <etl/message_bus.h>
#include <etl/message_router.h>


namespace fx
{
   /** Access the root dispatcher */
   etl::imessage_bus &get_root_dispatcher();

   /** Access the root dispatcher */
   void set_root_dispatcher( etl::imessage_bus * );

   /**
    * The Root dispatcher is a message bus of a given size
    * It overloads the << operator for convenience
    * It behaves like a singleton
    */
   template<const size_t MAXBUSES>
   struct RootDispatcher : public etl::message_bus<MAXBUSES>
   {
      RootDispatcher() { set_root_dispatcher( this ); }

      template<class T>
      RootDispatcher &operator<<( T &d )
      {
         this->subscribe( d );
         return *this;
      }
   };

   /** Publish a message at the root dispatcher */
   void publish( const etl::imessage &msg_ );

   /** Allow creating unique auto-incrementing routers ID */
   static inline etl::message_router_id_t message_router_auto_id{ 0 };

   /** Special ID for the dispatcher started */
   constexpr etl::message_id_t DISPATCHER_STARTED{ 255 };
   constexpr etl::message_id_t CHECK_HEATH{ 254 };

   /** Create the type for dispatcher started message */
   struct DispatcherStarted : etl::message<DISPATCHER_STARTED>
   {};

/** Allow a lookup of messages - for posix only */
#ifdef _POSIX
   void register_message( const char *name, size_t value );
#endif

   /**
    * Helper struct to create unique message types
    * Simply inherit specifying the name which can be used for debug
    * The name must be a typestring. C++20 concepts to the rescue!
    */
   template<class TName, const size_t TValue>
   struct Message : public etl::message<TValue>
   {
      constexpr const char *name() { return TName::data(); }
#ifdef _POSIX
      Message<TName, TValue>() { register_message( name(), TValue ); }
#endif
   };

/**
 * Helper macro to make the declaration even lighter
 */
#define FX_MSG( x ) struct x : public fx::Message<typestring_is( #x ), __COUNTER__>

   template<class T, class... TMsgs>
   struct Worker : public etl::message_router<T, TMsgs...>
   {
      Worker<T, TMsgs...>() : etl::message_router<T, TMsgs...>( ++message_router_auto_id ) {}

      void on_receive_unknown( const etl::imessage &msg ) {}
   };

   template<
      class TPacket,
      class TName,
      const size_t        STACKSIZE,
      const uint_least8_t MAX_ROUTERS = 1,
      const size_t        QUEUESIZE   = 4>
   class Dispatcher : public etl::message_bus<MAX_ROUTERS>
   {
      rtos::Queue<TPacket, QUEUESIZE> queue;
      rtos::Task<TName, STACKSIZE>    task;

   public:
      Dispatcher()
         : etl::message_bus<MAX_ROUTERS>()
         , task( etl::delegate<void()>::create<Dispatcher, &Dispatcher::run>( *this ) )
      {}

      template<class T>
      Dispatcher &operator<<( T &w )
      {
         this->subscribe( w );
         return *this;
      }

   protected:
      void receive( const etl::imessage &msg_ ) override
      {
         auto packet = TPacket( msg_ );
         queue.send( packet );
      }

      void run()
      {
         // Thread is started. Let all worker know it
         etl::message_bus<MAX_ROUTERS>::receive(
            etl::imessage_router::ALL_MESSAGE_ROUTERS, DispatcherStarted{} );

         while ( true )
         {
            auto packet = TPacket();

            queue.receive( packet );
            auto &msg = packet.get();
            etl::message_bus<MAX_ROUTERS>::receive(
               etl::imessage_router::ALL_MESSAGE_ROUTERS, msg );
         }
      }
   };
}  // namespace fx


#endif /* ndef fx_hpp_was_included */