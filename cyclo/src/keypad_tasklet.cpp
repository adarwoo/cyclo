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

/**
 * The tasklet translates the interrupt callback into a task callback.
 * The timer task (which runs at the highest priority) is used to do so.
 * The resulting tasklet simply posts a message into the FX framework.
 *
 * @author guillaume.arreckx
 */
#include "keypad_tasklet.hpp"

#include "asx.h"
#include "keypad.h"
#include "msg_defs.hpp"

#include <fx.hpp>
#include <logger.h>


namespace
{
   const char *const DOM = "keypad";
}


KeypadTasklet::KeypadTasklet()
{
   keypad_init();
   keypad_register_callback( KEY_UP | KEY_DOWN | KEY_SELECT, callback_from_isr, this );
}

void KeypadTasklet::callback_from_isr( uint8_t k, void *param )
{
   LOG_HEADER( DOM );

   KeypadTasklet *this_ = (KeypadTasklet *)param;
   this_->schedule_from_isr( (uint32_t)k );
}

void KeypadTasklet::run( uint32_t key )
{
   LOG_HEADER( DOM );

   msg::Keypad msg;
   msg.key_code = key;

   fx::publish( msg );
}
