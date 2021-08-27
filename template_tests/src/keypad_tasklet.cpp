/*
 * keypad_tasklet.cpp
 *
 * Created: 24/08/2021 00:34:33
 *  Author: micro
 */
#include <logger.h>
#include <fx.hpp>

#include "asx.h"
#include "keypad_tasklet.hpp"
#include "keypad.h"
#include "msg_defs.hpp"

namespace 
{
   const char * const DOM = "keypad";
}


KeypadTasklet::KeypadTasklet()
{
   keypad_init();
   keypad_register_callback(
      KEY_UP | KEY_DOWN | KEY_SELECT,
      callback_from_isr,
      this
   );
}

void KeypadTasklet::callback_from_isr(uint8_t k, void *param)
{
   LOG_HEADER(DOM);

   KeypadTasklet *this_ = (KeypadTasklet *)param;
   #ifdef _POSIX
   this_->schedule((uint32_t)k);
   #else
   this_->schedule_from_isr((uint32_t)k);
   #endif
}

void KeypadTasklet::run(uint32_t key)
{
   LOG_HEADER(DOM);

   msg::Keypad msg;
   msg.key_code = key;

   fx::publish(msg);
}
