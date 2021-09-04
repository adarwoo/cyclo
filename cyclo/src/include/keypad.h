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
#ifndef keypad_h_was_included
#define keypad_h_was_included

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

   /** Callback called when a key is pushed */
   typedef void ( *keypad_handler_t )( uint8_t key, void *param );

   /** Initialise the keypad library */
   void keypad_init( void );

   /**
    * Regsiter a callback for one or many key
    * key_masks Mask of the keys to handle
    * handler Callback function (from isr)
    * param Optional data to pass along
    */
   void keypad_register_callback( uint8_t key_masks, keypad_handler_t handler, void *param );

#ifdef __cplusplus
}
#endif

#endif /* ndef keypad_h_was_included */
