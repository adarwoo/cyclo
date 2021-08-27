/**
 * @ingroup generic
 * @{
 * @file
 * Implementation of the stack_tracer API
 * @author gax
 */
#include <stdint.h>
#include <string.h>

#include "logger.h"

void _log_trace_stack()
{
   LOG_ERROR( 0, "Call stack not available on this platform" );
}

