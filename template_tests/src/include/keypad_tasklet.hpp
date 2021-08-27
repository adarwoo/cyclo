#ifndef keypad_tasklet_hpp__included
#define keypad_tasklet_hpp__included

#include "rtos.hpp"

/**
 * Tasklet which posts the key event to the UI router
 */
class KeypadTasklet : public rtos::Tasklet
{
public:
    explicit KeypadTasklet();

    static void callback_from_isr(uint8_t k, void *param);

    virtual void run(uint32_t key) override;
};


#endif // ndef keypad_tasklet_hpp__included