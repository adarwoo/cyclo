#ifndef nonc_tasklet_hpp__included
#define nonc_tasklet_hpp__included

#include "asf.h"
#include "fx.hpp"
#include "msg_defs.hpp"
#include "cyclo.hpp"

#ifndef NONC_SAMPLE_FREQUENCY_HZ
#define NONC_SAMPLE_FREQUENCY_HZ 100
#endif

/**
 * Tasklet which posts the status of the NoNc switch
 *  and reports further changes as message
 */
class NoNcTasklet : public rtos::Tasklet
{
    enum class State
    {
        unknown,
        no,
        nc
    };
    
    inline static State state = State::unknown;
    inline static NoNcTasklet *this_ = nullptr;

public:
    NoNcTasklet()
    {
       // Remember this_ since the callback does have any params
       this_ = this;
          
        // Initialise the sampling timer
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
        bool nc_readback = ioport_get_pin_level(SWITCH_SENSE_NC);
        bool no_readback = ioport_get_pin_level(SWITCH_SENSE_NO);

        // Measure is viable?
        if ( nc_readback != no_readback )
        {
           // Any changes?
           cyclo::rocker_toggle_t now = nc_readback ? cyclo::rocker_toggle_t::nc : cyclo::rocker_toggle_t::no;
           
           if ( (not cyclo::toggle_pos) or (now != *cyclo::toggle_pos) )
           {
              cyclo::toggle_pos = now;
              assert(this_);
              this_->schedule_from_isr(0);
           }
       }
    }

    virtual void run(uint32_t unused) override
    {
       // Store globally
       fx::publish(msg::NoNcUpdate{});
    }
};


#endif // ndef nonc_tasklet_hpp__included