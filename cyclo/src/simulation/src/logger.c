#include <logger.h>
#include <logger/logger_os.h>

#include <FreeRTOS.h>
#include <task.h>

#include "asx.h"

//
// Logger - enhance
//

/**
 * Get the name of the thread instead
 */
void _log_copy_thread_id( char *buffer, size_t count, _LOG_THREAD_ID id )
{
   TaskHandle_t h = xTaskGetCurrentTaskHandle();

   if ( h != 0 )
   {
      snprintf( buffer, count, " %s ", pcTaskGetName( h ) );
   }
   else
   {
      snprintf( buffer, count, " main " );
   }
}
