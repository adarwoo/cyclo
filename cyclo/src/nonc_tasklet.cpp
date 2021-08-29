/*
 * nonc_tasklet.cpp
 *
 * Created: 23/08/2021 23:14:40
 *  Author: micro
 */
#include "nonc_tasklet.hpp"


#ifdef _POSIX
NoNcTasklet::NoNcTasklet( Contact &contact ) : contact{ contact }
{}

void NoNcTasklet::read_nonc()
{}
#else
#   include <asf.h>

NoNcTasklet::NoNcTasklet( Contact &contact ) : contact{ contact }
{
   // Remember this_ since the callback does have any params
   this_ = this;

   // Initialize the sampling timer
   tc_enable( &NONC_TC );
   tc_set_wgm( &NONC_TC, TC_WG_NORMAL );
   tc_set_resolution( &NONC_TC, sysclk_get_per_hz() / NONC_SAMPLE_FREQUENCY_HZ );
   tc_write_period( &NONC_TC, tc_get_resolution( &NONC_TC ) / NONC_SAMPLE_FREQUENCY_HZ );
   tc_set_overflow_interrupt_callback( &NONC_TC, &NoNcTasklet::read_nonc );

   // Low interrupt priority
   tc_set_overflow_interrupt_level( &NONC_TC, TC_INT_LVL_LO );
}

void NoNcTasklet::read_nonc()
{
   // Both sides are read
   // The state changes once the following is measured
   bool nc_readback = not ioport_get_pin_level( SWITCH_SENSE_NC );
   bool no_readback = not ioport_get_pin_level( SWITCH_SENSE_NO );

   // Measure is viable?
   if ( nc_readback != no_readback )
   {
      // First time read or any changes?
      if ( ! initialised || ( no_readback != was_no ) )
      {
         initialised = true;
         was_no      = no_readback;

         // Notify the initial status
         assert( this_ );
         this_->schedule_from_isr( no_readback );
      }
   }
}

#endif