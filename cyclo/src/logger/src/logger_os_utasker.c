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

#include <string.h>

// uTasker sole function declared as compatible extern to avoid header dependency
extern unsigned short fnDebugMsg(char *ucToSend);

#ifdef _WINDOWS
#include <Windows.h>
#endif


//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

/** Remember the last time the timestamp was displayed */
static time_t _log_last_timestamp;


//----------------------------------------------------------------------------
//  Implement OS specific functions
//----------------------------------------------------------------------------

/** Get hold of the current thread id */
_LOG_THREAD_ID _log_get_current_thread_id()
   { return 0; }

void _log_mutex_init()
{}

void _log_lock()
{}

void _log_unlock()
{}


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
   fnDebugMsg((char *)string);
   fnDebugMsg("\r\n");
#ifdef _WINDOWS
   OutputDebugStringA(string);
   // Windows needs a carriage return too
   OutputDebugStringA("\r");
#endif
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
bool log_os_parse_config( char *env_log)
{
   return true;
}


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
