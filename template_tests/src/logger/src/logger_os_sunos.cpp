/*-
 *****************************************************************************
 * SunOs implementation for the debug library
 *
 * @author gpa
 *****************************************************************************
 */

#include "logger.h"
#include "logger/logger_os.h"
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

/** The log lock for access to container etc.. */
static pthread_mutex_t _log_mutex;

/** Remember the last time the timestamp was displayed */
static time_t _log_last_timestamp;

/** Flag to indicate that log exit function was called */
static bool _log_exited = false;

//----------------------------------------------------------------------------
//  Implement OS specific functions
//----------------------------------------------------------------------------
/** Get hold of the current thread id */
_LOG_THREAD_ID _log_get_current_thread_id()
   { return (_LOG_THREAD_ID)pthread_self(); }

/** Return a string with the debug configuration */
const char *log_get_config_string()
   { return getenv("LOG"); }

#ifdef _REENTRANT
/** Create a portable mutex */
void _log_mutex_init()
   { pthread_mutex_init( &_log_mutex, 0 ); }

/** Lock log mutex which protects printf operations */
void _log_lock()
   { pthread_mutex_lock( &_log_mutex ); }

/** Unlock log mutex */
void _log_unlock()
   { pthread_mutex_unlock( &_log_mutex ); }
#else
void _log_mutex_init() {}
void _log_lock()       {}
void _log_unlock()     {}
#endif

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
   if ( _log_outfile == 0 )
   {
      int priority;

      switch ( _log_type )
      {
         case LOG_LEVEL_ERROR :
         case LOG_LEVEL_WARN  : priority = LOG_ERR;    break;
         case LOG_LEVEL_MILE  : priority = LOG_NOTICE; break;
         case LOG_LEVEL_TRACE : priority = LOG_INFO;   break;
         default:               priority = LOG_DEBUG;  break;
      }

      syslog( priority, "%s", string );
   }
   else
   {
      fprintf(_log_outfile, "%s\n", string);
   }
}


/**
 *****************************************************************************
 * Returns a string with a timestamp or 0 if the time has not changed
 *  sufficiently.
 *
 * @param epoch Optional number of seconds since Jan 1970. If 0, the
 *              function must get the local time from the OS.
 * @Return The date time string to print
 *****************************************************************************
 */
const char *log_os_timestamp(unsigned long epoch)
{
   static char acTimeStamp[ 32 ];

   // Get current time
   time_t CurrentTime;

   if ( epoch )
   {
      CurrentTime = epoch;
   }
   else
   {
      CurrentTime = time( 0 );
   }

   // Has more than one second elapsed since the last output
   if ( CurrentTime != _log_last_timestamp )
   {
      // Time has changed so include it in this report
      memset(acTimeStamp,0,sizeof(acTimeStamp));
      _log_last_timestamp = CurrentTime;
      struct tm CurrentTime_tm;
      localtime_r(&CurrentTime,&CurrentTime_tm);
      strftime( acTimeStamp, sizeof(acTimeStamp), "- %d/%m/%Y %H:%M:%S\n", &CurrentTime_tm );
      return acTimeStamp;
   }

   return 0;
}


/**
 *****************************************************************************
 * Parse os specific parameters
 * @param  arg   String with argument to parse
 *
 * @param string   The string to print
 *****************************************************************************
 */
bool log_os_parse_config( char *env_log)
{
   if ( strcmp( env_log, "syslog" ) == 0 )
   {
      // Flag that we are using an alternative log system
      _log_outfile = 0;

      // Use the process name when writting to syslog
      openlog( _log_get_process_name(), LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER );
   }
   else
   {
      return false;
   }

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
   strncpy(_log_process_name, "LOG", maxLength);

   // Get hold of the process name
   pid_t pid = getpid();

   if ( pid > 0 )
   {
      char buf[64] = {0};
      snprintf(buf, sizeof(buf) - 1, "/proc/%d/status", pid);

      // Locate the process name from the proc interface !!
      FILE *pf = fopen( buf, "r" );

      if ( pf != NULL )
      {
         // Usually, the process name is the first line
         while ( ! feof(pf) )
         {
            if ( fgets( buf, sizeof(buf), pf ) )
            {
               if ( strncmp( "Name:", buf, 5 ) == 0 )
               {
                  char *pProcessName = &buf[5];

                  // Locate actual start of the name
                  while ( *pProcessName==' ' || *pProcessName=='\t' )
                  {
                     pProcessName++;
                  }

                  size_t pslen = strlen( pProcessName );

                  if ( pslen )
                  {
                     if ( pslen > maxLength )
                     {
                        // Process name is too long: strip process name
                        memcpy(pProcessName + maxLength - 4, "...", 3);
                        pProcessName[maxLength - 1] = '\0';
                     }
                     else
                     {
                        // Remove trailing CR
                        pProcessName[pslen - 1] = '\0';
                     }

                     // Copy the process name
                     strncpy( _log_process_name, pProcessName, maxLength );
                  }

                  break;
               }
            }
         }

         fclose(pf);
      }
   }
}


/* ---------------------------- End of file ------------------------------- */

