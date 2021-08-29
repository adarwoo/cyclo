#ifndef nonc_tasklet_hpp__included
#define nonc_tasklet_hpp__included

#include "contact.hpp"


#ifndef NONC_SAMPLE_FREQUENCY_HZ
#   define NONC_SAMPLE_FREQUENCY_HZ 100
#endif

/**
 * Tasklet which posts the status of the NoNc switch
 *  and reports further changes as message
 */
class NoNcTasklet : public rtos::Tasklet
{
   inline static NoNcTasklet *this_ = nullptr;

   // Cache the last result
   inline static bool was_no = true;

   ///< Wait for a first status
   inline static bool initialised = false;

   using no_nc_t = Contact::no_nc_t;

   // Contact
   Contact &contact;

public:
   NoNcTasklet( Contact &contact );

   static void read_nonc();

   virtual void run( uint32_t no_readback ) override
   {
      // Get the manager to take care of it
      contact.set_as_no( no_readback );
   }
};


#endif  // ndef nonc_tasklet_hpp__included