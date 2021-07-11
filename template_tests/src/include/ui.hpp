/*
 * fx.cpp
 *
 * Created: 04/07/2021 14:35:24
 *  Author: micro
 */ 
#include <alloca.h>

#include "etl/type_lookup.h"
#include "etl/static_assert.h"
#include "etl/parameter_pack.h"
#include "etl/shared_message.h"
#include "etl/message.h"
#include "etl/reference_counted_message_pool.h"
#include "etl/message_router.h"
#include "etl/message_bus.h"
#include "etl/fixed_sized_memory_block_allocator.h"
#include "etl/queue.h"

#include "typestring.hpp"
#include "rtos.hpp"

// Map string types literals to 'id'
using MessageTypes = etl::parameter_pack<
   typestring_is("no_nc_changed"),
   typestring_is("set_relay"),
   typestring_is("refresh_ui"),
   typestring_is("keypad")
>;

struct KeypadMsg : etl::message<MessageTypes::index_of_type_v<typestring_is("keypad")>>
{
   uint8_t key_code;
   bool long_push;
};

struct RockerSwitchMsg : etl::message<MessageTypes::index_of_type_v<typestring_is("no_nc_changed")>> 
{
   // No data. The value is atomic - better read it from the source 
};

struct RefreshUIMsg : etl::message<MessageTypes::index_of_type_v<typestring_is("refresh_ui")>>
{
   // Command message. No data
};

struct SetRelayMsg : etl::message<MessageTypes::index_of_type_v<typestring_is("set_relay")>>
{
   ///< Turn no of off (irrespective of the NO/NC status. True to turn ON.
   bool turn_on;
};

class UI : public etl::message_router<UI, RefreshUIMsg, KeypadMsg>
{
public:
   void on_receive(const RefreshUIMsg& msg) {}
   void on_receive(const KeypadMsg& msg) {}
   void on_receive_unknown(const etl::imessage& msg) {}
      
   UI() : message_router(1) {}
};
