/*-
 *****************************************************************************
 * Windows implementation for the tracing library
 *
 * @author gpa
 *****************************************************************************
 */


//----------------------------------------------------------------------------
//  Local include
//----------------------------------------------------------------------------
#include "logger.h"
#include "logger/logger_os.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <assert.h>


//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

/** The log lock for access to container etc.. */
static HANDLE _log_mutex;

/** Remember the last time the timestamp was displayed */
static time_t _log_last_timestamp;

/** Flag to indicate that log exit function was called */
static bool _log_exited = false;

//----------------------------------------------------------------------------
//  Implement OS specific functions
//----------------------------------------------------------------------------

/** Get hold of the current thread id */
_LOG_THREAD_ID _log_get_current_thread_id()
   { return (_LOG_THREAD_ID)GetCurrentThreadId(); }

/** Return a string with the debug configuration */
const char *log_get_config_string()
   { return getenv("LOG"); }

void _log_mutex_init()
{
   _log_mutex = CreateMutex(NULL,FALSE,NULL);
   assert(_log_mutex!=NULL);
}

void _log_lock()
{
   DWORD dwResult;
   dwResult = WaitForSingleObject( _log_mutex, INFINITE );
   assert( dwResult == WAIT_OBJECT_0 );
}

void _log_unlock()
{
   BOOL bSuccess;
   bSuccess = ReleaseMutex( _log_mutex );
   assert( bSuccess != FALSE );
}

/**
 *****************************************************************************
 * Cleaning the debugging library
 *****************************************************************************
 */
void _log_exit( void )
{
   if ( ! _log_exited )
   {
      _log_exited = true;
   }
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
   if ( _log_outfile )
   {
      fprintf(_log_outfile, "%s\n", string);
   }
   else
   {
#ifdef _CONSOLE
      puts(string);
      puts("\n\r");
#else
      _CrtLogReport( _CRT_WARN, 0, 0, 0, string );
#endif
   }
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
   return false;
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
   strcpy(_log_process_name, "LOG");
}


/* ---------------------------- End of file ------------------------------- */

