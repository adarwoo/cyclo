#ifndef nonc_tasklet_hpp__included
#define nonc_tasklet_hpp__included

#include <asf.h>

#include "contact_manager.hpp"


#ifndef NONC_SAMPLE_FREQUENCY_HZ
#define NONC_SAMPLE_FREQUENCY_HZ 100
#endif

/**
 * Tasklet which posts the status of the NoNc switch
 *  and reports further changes as message
 */
class NoNcTasklet : public rtos::Tasklet
{
    inline static NoNcTasklet *this_ = nullptr;
    inline static ContactManager& contact{ContactManager::instance()};
    
    using no_nc_t = ContactManager::no_nc_t;

public:
    NoNcTasklet()
    {
       // Remember this_ since the callback does have any params
       this_ = this;
          
       // Initialize the sampling timer
       tc_enable(&NONC_TC);
       tc_set_wgm(&NONC_TC, TC_WG_NORMAL);
       tc_set_resolution(&NONC_TC, sysclk_get_per_hz() / NONC_SAMPLE_FREQUENCY_HZ);
       tc_write_period(&NONC_TC, tc_get_resolution(&NONC_TC) / NONC_SAMPLE_FREQUENCY_HZ);
       tc_set_overflow_interrupt_callback(&NONC_TC, &NoNcTasklet::read_nonc);

       // Low interrupt priority
       tc_set_overflow_interrupt_level(&NONC_TC, TC_INT_LVL_LO);
    }

    static void read_nonc()
    {
        // Both sides are read
        // The state changes once the following is measured
        bool nc_readback = not ioport_get_pin_level(SWITCH_SENSE_NC);
        bool no_readback = not ioport_get_pin_level(SWITCH_SENSE_NO);

        // Measure is viable?
        if ( nc_readback != no_readback )
        {
           // Any changes?
           if ( no_readback != contact.is_no() )
           {
              assert(this_);
              this_->schedule_from_isr(no_readback);
           }
       }
    }

    virtual void run(uint32_t no_readback) override
    {
       // Get the manager to take care of it
       contact.set_as_no(no_readback);
    }
};


#endif // ndef nonc_tasklet_hpp__included