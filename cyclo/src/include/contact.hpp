#ifndef contact_h_included
#define contact_h_included
/*
 * contact.h
 *
 * Created: 06/08/2021 22:00:45
 *  Author: software@arreckx.com
 */

#include <cstdint>

#include <fx.hpp>

#include "msg_defs.hpp"


class Contact
{
   friend class NoNcTasklet;

public:
   enum no_nc_t : uint8_t { no, nc };

   enum contact_t : uint8_t { leave_as, open, close };

private:
   ///< Connection type
   no_nc_t type;

   ///< State of the contact (not the relay)
   contact_t contact;

public:
   Contact() : type{ no }, contact{ leave_as } {}

   ///< Accessors
   inline bool is_no() const { return type == no; }

   ///< @return true if the contact is opened
   bool is_open() const;

   ///< Flip the contact
   inline void flip()
   {
      // Flip
      set( is_open() ? close : open );

      // Further changes to NO/NC won't toggle the relay
      set( leave_as );
   }

   ///< Control the relay
   void set( contact_t v )
   {
      auto old_contact = contact;
      contact          = v;

      switch ( v )
      {
      case open: set_relay( ! is_no() ); break;
      case close: set_relay( is_no() ); break;
      case leave_as:
      default: break;
      }

      if ( old_contact != contact )
      {
         // Update the GUI
         fx::publish( msg::ContactUpdate{} );
      }
   }

protected:
   /**
    * Called by the tasklet when the no nc has changed
    * Change the relay accordingly, and notify the GUI
    */
   inline void set_as_no( bool is_no )
   {
      bool prev = is_open();

      this->type = is_no ? no : nc;
      set( contact );

      if ( prev != is_open() )
      {
         fx::publish( msg::ContactUpdate{} );
      }

      fx::publish( msg::NoNcUpdate{} );
   }

   ///< Control the relay
   void set_relay( bool close_contact );
};


#endif /* ndef contact_manager_h_included */