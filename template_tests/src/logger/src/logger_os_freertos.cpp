/*-
 *****************************************************************************
 * Linux implementation for the debug library
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
//  Local types
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------
/** The log lock for access to container etc.. */
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
static pthread_mutex_t _log_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
static pthread_mutex_t _log_mutex;
#endif

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
   {
   #ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
      pthread_mutexattr_t attrib;
      pthread_mutexattr_init(&attrib);
      pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE_NP);
      pthread_mutex_init( &_log_mutex, &attrib );
   #endif
   }

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

#ifdef LOG_THREADED_LOG
/* Message queue */
extern void stop_log_thread(void);
extern int push_syslog_log_to_queue(const char * string, int priority);
extern int push_file_log_to_queue(const char * string, FILE * file);
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
#ifdef LOG_THREADED_LOG
      stop_log_thread();
#endif
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
         case LOG_LEVEL_INFO  : priority = LOG_INFO;   break;
         case LOG_LEVEL_TRACE :
         case LOG_LEVEL_DEBUG :
         default:               priority = LOG_DEBUG;  break;
      }

#ifdef LOG_THREADED_LOG
      /* Push log to queue */
      push_syslog_log_to_queue(string, priority);
#else
      /* Log message */
      syslog( priority, "%s", string );
#endif
   }
   else
   {
#ifdef LOG_THREADED_LOG
      /* Push log to queue */
      push_file_log_to_queue(string, _log_outfile);
#else
      /* Write message to file */
      fprintf(_log_outfile, "%s\n", string);
#endif
   }
}


/**
 *****************************************************************************
 * Returns a string with a timestamp or 0 if the time has not changed
 *  sufficiently.
 *
 * @param epoch Optional number of seconds since Jan 1970. If 0, the
 *              function must get the local time from the OS.
 * @return   String with the timestamp
 *****************************************************************************
 */
const char *log_os_timestamp(unsigned long epoch)
{
   static char acTimeStamp[32] = {0};
   time_t currentTime;

   // Get current time
   if ( epoch )
   {
      currentTime = time_t(epoch);
   }
   else
   {
      currentTime = time( 0 );
   }

   // Has more than one second elapsed since the last output
   if ( currentTime != _log_last_timestamp )
   {
      // Time has changed so include it in this report
      memset(acTimeStamp,0,sizeof(acTimeStamp));
      _log_last_timestamp = currentTime;
      struct tm currentTime_tm;
      localtime_r(&currentTime,&currentTime_tm);
      strftime( acTimeStamp, sizeof(acTimeStamp), "- %Y/%m/%d %H:%M:%S\n", &currentTime_tm );
      return acTimeStamp;
   }

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


/**
 * Get the name of the thread instead
 */
extern "C" void _log_copy_thread_id( char *buffer, size_t count, _LOG_THREAD_ID id)
{
   pthread_getname_np(id, buffer, count);

   if ( *buffer == 0 )
   {
      snprintf( buffer, count, " 0x%lx ", id );
   }
}


/* ---------------------------- End of file ------------------------------- */

