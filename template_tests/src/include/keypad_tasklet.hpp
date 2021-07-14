#ifndef keypad_tasklet_hpp__included
#define keypad_tasklet_hpp__included

#include "rtos.hpp"
#include "etl/message_bus.h"
#include "msg_defs.hpp"
#include "keypad.h"

/**
 * Tasklet which posts the key event to the UI router
 */
class KeypadTasklet : public rtos::Tasklet
{
    etl::imessage_bus &root_dispatcher;

public:
    KeypadTasklet(etl::imessage_bus &root) : root_dispatcher(root)
    {
        keypad_init();
        keypad_register_callback(
            KEY_UP | KEY_DOWN | KEY_SELECT,
            callback_from_isr,
            this);
    }

    static void callback_from_isr(uint8_t k, void *param)
    {
        KeypadTasklet *this_ = (KeypadTasklet *)param;
        this_->schedule_from_isr((uint32_t)k);
    }

    virtual void run(uint32_t key) override
    {
        msg::Keypad msg;
        msg.key_code = key;

        //etl::send_message(root_dispatcher, msg);
        root_dispatcher.receive(msg);
    }
};


#endif // ndef keypad_tasklet_hpp__included