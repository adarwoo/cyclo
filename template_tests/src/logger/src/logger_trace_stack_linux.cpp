/**
 * @ingroup generic
 * @{
 * @file
 * Implementation of the stack_tracer API
 * @author gax
 */
#include <execinfo.h>
#include <cxxabi.h>
#include <stdint.h>
#include <string.h>

#include "logger.h"

#ifdef LOG_THREADED_LOG
/* Message queue */
extern void flush_log_queue(bool logAllMsg);
#endif

namespace
{
   /** Max depth of stack dump */
   static const size_t max_depth = 50;

   /** Max length of a demangled symbol */
   static size_t max_symbol_length = 100;

   /**
    * Take a stack string, demangle it and send it to the logger
    */
   void demangle_and_print( const char *stack_string )
   {
      char *function = (char *)malloc(max_symbol_length);
      char *begin = 0, *end = 0;

      // create a local buffer, don't modify the original stack string
      char *stack_string_buf = strdup(stack_string);

      // find the parentheses and address offset surrounding the mangled name
      for (char *j = stack_string_buf; *j; ++j)
      {
         if (*j == '(')
         {
            begin = j;
         }
         else if (*j == '+')
         {
            end = j;
         }
      }

      if (begin && end)
      {
         int status;

         // found our mangled name, now in [begin, end)
         *begin = '\0';
         ++begin;
         *end = '\0';

         char *ret = abi::__cxa_demangle(begin, function, &max_symbol_length, &status);

         if (ret)
         {
            // return value may be a realloc() of the input
            function = ret;
         }
         else
         {
            // demangling failed, just pretend it's a C function with no args
            strncpy(function, begin, max_symbol_length);
            strncat(function, "()", max_symbol_length);
            function[max_symbol_length - 1] = '\0';
         }

         LOG_ERROR( 0, "    %s:%s", stack_string_buf, function );
      }
      else
      {
         // didn't find the mangled name, just print the whole line
         LOG_ERROR( 0, "    %s", stack_string_buf);
      }

      free(stack_string_buf);
      free(function);
   }
}


void _log_trace_stack()
{
   size_t stack_depth;
   void *stack_addrs[max_depth];
   char **stack_strings;
   char * prev_stack_string = 0;

   // Note: on some targets (e.g. arm), backtrace unwinding may not work properly with exceptions
   // causing unwind loop with the following side effect:
   // The highest frame in stack is returned N times
   // where N is the number of unused string slots left (N = max_depth - real stack_depth)
   // So stack_depth is always equal to max_depth when occurs
   stack_depth = backtrace(stack_addrs, max_depth);
   stack_strings = backtrace_symbols(stack_addrs, stack_depth);

   LOG_ERROR( 0, "Call stack" );

   for (size_t i = 1; i < stack_depth; i++)
   {
      if ( (prev_stack_string == 0) || (strcmp(stack_strings[i], prev_stack_string) != 0) )
      {
         // Print stack string only if different than the previous one.
         // This will fix unwind loop side effect
         demangle_and_print( stack_strings[i] );

         // Store for next stack string
         prev_stack_string = stack_strings[i];
      }
   }

   free(stack_strings); // malloc()ed by backtrace_symbols


#ifdef LOG_THREADED_LOG
   // If thread is used for internal logging, flush all the logs
   flush_log_queue(true);
#endif
}

