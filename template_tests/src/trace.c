/*
 * trace.c
 *
 * Created: 09/08/2021 20:36:12
 *  Author: micro
 */
#include <avr/io.h>

#include <asf.h>
#include <alloca.h>
#include <string.h>

#include "trace.h"


void trace_inspect(const void *p, size_t len) 
{
   void * c = alloca (len);
   memset(c, 0xAA, len);
   memcpy(c, p, len);
   trace_set(TRACE_INFO);
}


void trace_assert(bool test, ioport_pin_t pin)
{
   if ( ! test )
   {
      trace_set(pin);

      while (1)
      {
         asm("break");
         continue;
      }
   }
}

/**
 * Override the exit handler
 */
void exit_handler (void) __attribute__ ((naked)) \
__attribute__ ((section (".fini9")))
__attribute__ ((used));
void
exit_handler (void)
{
   trace_set(TRACE_ERR);
   asm("break");
   for (;;);
}
