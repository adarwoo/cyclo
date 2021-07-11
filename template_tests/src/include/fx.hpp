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


template<class... Ts>
struct dispatcher : public etl::message_bus<4>
{
public:
   dispatcher(const size_t s) : etl::message_bus<4>() {}
};


template<class T, class... Ts>
struct dispatcher<T, Ts...> : public dispatcher<Ts...>
{
   T tail;
public:     
   dispatcher() : dispatcher<Ts...>(1), tail(sizeof...(Ts)) {}
   dispatcher(const size_t s) : dispatcher<Ts...>(s+1), tail(sizeof...(Ts)) {}
};


#endif /* FX_H_ */