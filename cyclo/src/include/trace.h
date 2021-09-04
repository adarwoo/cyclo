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
#ifndef trace_h_was_included
#define trace_h_was_included
/*
 * trace.h
 *
 * Created: 26/04/2020 22:39:15
 *  Author: micro
 */

#include <stddef.h>
#include "asx.h"


static inline void trace_set( ioport_pin_t pin )
{
   ioport_set_pin_level( pin, true );
}
static inline void trace_clear( ioport_pin_t pin )
{
   ioport_set_pin_level( pin, false );
}
static inline void trace_tgl( ioport_pin_t pin )
{
   ioport_toggle_pin_level( pin );
}

#ifdef __cplusplus
extern "C"
#endif
void trace_assert( bool test, ioport_pin_t pin );

// Overwrite the assert macro
// TRACE_ERR must be configured
#undef assert
#define assert( cond ) trace_assert( cond, TRACE_ERR )

#ifdef __cplusplus
/**
 * RAII style tracing
 */
struct Trace
{
   ioport_pin_t pin;

   Trace( const ioport_pin_t pin ) : pin( pin ) { trace_set( pin ); }

   ~Trace() { trace_clear( pin ); }
};
#endif

#ifdef __cplusplus
extern "C"
#endif
void trace_inspect( const void *p, size_t len );


#endif /* ndef trace_h_was_included */