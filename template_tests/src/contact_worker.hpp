#ifndef contact_worker_hpp__included
#define contact_worker_hpp__included
/*
 * UI Woker
 */
#include "fx.hpp"
#include "msg_defs.hpp"
#include "cyclo.hpp"


class ContactWorker : public fx::Worker<ContactWorker, msg::SetRelay, msg::NoNcUpdate>
{
   void set()
   {
      using namespace cyclo;
      
      // Get the status
      bool close =
         ((contact == contact_t::closed) and ( toggle_pos == rocker_toggle_t::no))
         or
         ((contact == contact_t::opened) and ( toggle_pos == rocker_toggle_t::nc));
      
      ioport_set_pin_level(RELAY_CTRL, close);
   }
public:
   void on_receive(const msg::SetRelay &msg)
   {
      set();
   }

   void on_receive(const msg::NoNcUpdate &msg)
   {
      set();
   }
};


#endif // ndef contact_worker_hpp__included
