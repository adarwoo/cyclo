/*
 * fx.cpp
 *
 * Created: 25/08/2021 11:36:06
 *  Author: micro
 */ 
#include <logger.h>
#include <fx.hpp>

#ifdef _POSIX
#  include <map>
#endif

namespace fx
{
   namespace
   {
      // Debug domain
      const char * const DOM = "fx";
   
      // The unique instance of the root dispatcher
      etl::imessage_bus *root_dispatcher = nullptr;

      #ifdef _POSIX
         std::map<size_t, const char *> messages_lookup_map {};
      #endif
   }

   #ifdef _POSIX
      void register_message(const char *name, size_t value)
      {
         messages_lookup_map[value] = name;
      }
   #endif
   

   /** Access the root dispatcher */
   etl::imessage_bus &get_root_dispatcher()
   {
      assert(root_dispatcher);
   
      return *root_dispatcher;
   }

   /** Access the root dispatcher */
   void set_root_dispatcher(etl::imessage_bus *bus)
   {
      LOG_DEBUG(DOM, "Setting a new root dispatcher");
      assert(root_dispatcher == nullptr);
   
      root_dispatcher = bus;
   }

   void publish(const etl::imessage& msg_)
   {
      #ifdef _POSIX
      LOG_DEBUG(DOM, "Publishing: %s [%d]", messages_lookup_map[msg_.get_message_id()], msg_.get_message_id());
      #endif
   
      /** The instance of the root dispatcher. There can only be one */
      assert(root_dispatcher);
   
      root_dispatcher->receive(msg_);
   }
}
