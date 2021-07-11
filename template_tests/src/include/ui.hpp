#ifndef ui_hpp__included
#define ui_hpp__included
/*
 * User interface message router
 * Created: 04/07/2021 14:35:24
 *  Author: micro
 */ 
#include "etl/message_router.h"
#include "etl/function.h"
#include "etl/delegate.h"

#include "msg_defs.hpp"
#include "trace.h"
#include "rtos.hpp"

#if 0
template<class TString, class TPacket, class TRouter, const size_t TStackSize=128, const size_t TQueueSize=2>
class ServiceTask : public rtos::Task<TString, TStackSize>
{
   rtos::Queue<TPacket, TQueueSize> queue;
   TRouter router;
   ServiceTask *_this;
   
public:
   // Run the service task which reads from the queue and routes to the router.
   virtual void invoke()
   {
      for (;;)
      {
         TPacket packet;
         
         queue.receive(packet);
         router.receive(packet.get());
      }
   }
   
   template<class T> void send(T &message)
   {
      TPacket packet(message);
      queue.send(packet);
   }

   template<class T> void send_from_isr(T &message)
   {
      TPacket packet(message);
      queue.send_from_isr(packet);
   }
};


template <const size_t TBufferSize, typename TDerived,
   typename T1, typename T2 = void, typename T3 = void, typename T4 = void,
   typename T5 = void, typename T6 = void, typename T7 = void, typename T8 = void,
   typename T9 = void, typename T10 = void, typename T11 = void, typename T12 = void,
   typename T13 = void, typename T14 = void, typename T15 = void, typename T16 = void>
class Worker : public etl::message_router<Worker, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15,  T16>
{
   // Message buffer to queue to the handler
   rtos::MessageBuffer<TBufferSize> queue;
   
   // Override the base class's receive function.
   void receive(const etl::imessage& msg_)
   {
      if (accepts(msg_))
      {
         // Write in buffer - if space left!
         queue.send(&msg_, msg_.get_size());
      }
      else
      {
         assert(0);
      }
   }

   // Override the base class's receive function.
   void receive_from_isr(const etl::imessage& msg_)
   {
      if (accepts(msg_))
      {
         message_packet empack = msg_;
         
         // Write in buffer - if space left!
         queue.send_from_isr(&empack);
      }
      else
      {
         assert(0);
      }
   }

   void dispatch(const etl::imessage& msg)
   {
      const etl::message_id_t id = msg.get_message_id();

      switch (id)
      {
        case T1::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T1&>(msg)); break;
        case T2::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T2&>(msg)); break;
        case T3::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T3&>(msg)); break;
        case T4::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T4&>(msg)); break;
        case T5::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T5&>(msg)); break;
        case T6::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T6&>(msg)); break;
        case T7::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T7&>(msg)); break;
        case T8::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T8&>(msg)); break;
        case T9::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T9&>(msg)); break;
        case T10::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T10&>(msg)); break;
        case T11::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T11&>(msg)); break;
        case T12::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T12&>(msg)); break;
        case T13::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T13&>(msg)); break;
        case T14::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T14&>(msg)); break;
        case T15::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T15&>(msg)); break;
        case T16::ID: static_cast<TDerived*>(this)->on_receive(static_cast<const T16&>(msg)); break;
        default:
        {
            if (has_successor())
            {
                get_successor().receive(msg);
            }
            else
            {
               assert(0);
            }
            break;
         }
      }
   }      
      
   void process_queue()  
   {
      while (true)
      {
         message_packet& packet = 
         
         if ( queue.receive(packet) )
         {
            dispatch(packet);
         }
      }
   }
};


class Dispatcher : etl::message_bus<msg::router::size>
{
   
};


class UI : public Worker<UI, msg::Keypad>
{
public:
   void on_receive(const msg::Keypad &msg)
   {
      using namespace rtos::tick; // for _ms
      ioport_pin_t pin;

      switch (msg.key_code)
      {
         case KEY_UP: pin=TRACE_IDLE; break;
         case KEY_DOWN: pin=TRACE_TICK; break;
         case KEY_SELECT: pin=TRACE_ERR; break;
         default:
         assert(0);
      }
      
      trace_set(pin);
      rtos::delay(40_ms);
      trace_clear(pin);
   }
      
   void on_receive_unknown(const etl::imessage& msg)
   {
   }
   
   UI() : message_router(msg::router::UI) {}
};

using UIService = ServiceTask<typestring_is("ui"), UI>;






// Objective

class UIWorker : public Worker<UIWorker, RefreshMsg, ToggleMsg>
{
   void receive(const RefreshMsg &msg);
}

class ConsoleWorker : public Worker<UIWorker, OnUsbChar>
{
   void receive(const RefreshMsg &msg);
}

class SequencerWorker : public Worker<UIWorker, OnTimer>
{
   
}


using MainDispatcher = Dispatcher<ConsoleWorker, SequencerWorker>;
using UIDispatecher = Dispatcher<UIWorker>;

// Create the instance of the root dispatcher
auto root_dispatcher = RootDispatcher<MainDispatcher, UIDispatcher>();




#endif
#endif // ndef ui_hpp__included
