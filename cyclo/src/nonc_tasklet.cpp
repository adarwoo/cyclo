/******************************************************************************
The MIT License(MIT)
https://github.com/adarwoo/cyclo

Copyright(c) 2021 Guillaume ARRECKX - software@arreckx.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

/*
 * The tasklet object arms a timer IRQ called periodically to check the contact,
 *  and upon a change, delegates to the programs manager the handling
 *  of the change from the context of a tasklet.
 * @author software@arreckx.com
 */
#include "nonc_tasklet.hpp"

#include "asx.h"


NoNcTasklet::NoNcTasklet( Contact &contact ) : contact{ contact }
{
   #ifndef _POSIX
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
   #endif
}

void NoNcTasklet::read_nonc()
{
   #ifndef _POSIX
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
         // Get out of the IRQ quickly - and let the tasklet take over
         this_->schedule_from_isr( no_readback );
      }
   }
   #endif
}

void NoNcTasklet::run( uint32_t no_readback )
{
   // Get the contact manager to take care of it
   contact.set_as_no( no_readback );
}
