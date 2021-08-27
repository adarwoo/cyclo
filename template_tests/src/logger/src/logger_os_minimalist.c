/*-
 *****************************************************************************
 * uTasker implementation of the logger library.
 * Messages should be logger using uTasker API to eventually come
 *  out on a telnet or serial connection.
 * @author gpa
 *****************************************************************************
 */


//----------------------------------------------------------------------------
//  Local include
//----------------------------------------------------------------------------
#include "logger.h"
#include "logger/logger_os.h"
#include "logger/logger_limits.h"

#include <string.h>
#include <stdio.h>


//----------------------------------------------------------------------------
//  Local storage
//----------------------------------------------------------------------------
char log_config[_LOG_MAX_COMMAND_LINE_LENGTH] = "mile";


//----------------------------------------------------------------------------
//  Implement OS specific functions
//----------------------------------------------------------------------------

/** Get hold of the current thread id */
_LOG_THREAD_ID _log_get_current_thread_id()
   { return 0; }

/** Return a string with the debug configuration */
const char *log_get_config_string()
   { return log_config; }

void _log_mutex_init()
{
}

void _log_lock()
{
}

void _log_unlock()
{
}


/**
 *****************************************************************************
 * _log_print for NON ide versions
 * This function only take a string. The string should rely on function such
 *  as vsnprintf for their formatting. This is done since some tracing APIs
 *  can be very basic
 *
 * @param string   The string to print
 *****************************************************************************
 */
void _log_print(const char *string)
{
   puts(string);
}


/**
 *****************************************************************************
 * Returns a string with a timestamp or 0 if the time has not changed
 *  sufficiently.
 * @param epoch Optional number of seconds since Jan 1970. If 0, the
 *              function must get the local time from the OS.
 *
 * @return   String with the timestamp
 *****************************************************************************
 */
const char *log_os_timestamp( unsigned long t )
{
   return 0;
}


/**
 *****************************************************************************
 * Parse os specific parameters
 *
 * @param  env_log   Env string
 * @return true on success
 *****************************************************************************
 */


/**
 *****************************************************************************
 * Get hold of the process name and store in the global
 *  variable _log_process_name.
 * The method is called first. It is a good place for running other
 *  initialisations
 *
 *****************************************************************************
 */
void log_os_set_process_name( char *_log_process_name, size_t maxLength )
{
   // By default, the process is LOG
   strcpy(_log_process_name, "");
}


/* ---------------------------- End of file ------------------------------- */
