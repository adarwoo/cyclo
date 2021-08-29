 /*-
  *****************************************************************************
  * Linux implementation for the internal threaded logging library
  *
  * @author rsd
  *****************************************************************************
  */
#include "logger.h"
#include "logger/logger_os.h"
#include "logger/logger_limits.h"
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>

#ifdef LOG_THREADED_LOG

//----------------------------------------------------------------------------
//  Local defines and macros
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//  Local types
//----------------------------------------------------------------------------

typedef enum logMessageOutput
{
   /** Output is syslog */
   LOG_MSG_OUT_SYSLOG=0,
   /** Output is a file */
   LOG_MSG_OUT_FILE,
   /** Output is dummy (Message to be ignored) */
   LOG_MSG_OUT_DUMMY,

} logMessageOutput_t;


/**
 * Message
 */
typedef struct logMessage
{
   /** Type of the output */
   logMessageOutput_t output;

   /** File when logging to file */
   FILE * file;

   /** Syslog priority of the message */
   int syslog_priority;

   /** Message content (buffer) */
   char buffer[_LOG_SIZED_FOR(_LOG_MAX_TRACE)];

} _logMessage_t;


/**
 * Message queue.
 *
 * Queue used for buffering safely outgoing message when accessing to disk like flash
 */
typedef struct logMqueue
{
   _logMessage_t        messages[_LOG_MAX_MQUEUE_DEPTH];   /** Message buffer */
   _logMessage_t        *oldest_msg_ptr;  /** A pointer on the oldest message */
   _logMessage_t        *next_msg_ptr;    /** A pointer on the next message   */
   size_t               depth;            /** The size of the queue           */
   size_t               nb_of_msgs;       /** The number of messageS in queue */
   pthread_mutexattr_t  mutex_attr;       /** Mutex attribute                 */
   pthread_mutex_t      mutex;            /** Mutex                           */
   pthread_condattr_t   cond_attr;        /** Condition variable attribute    */
   pthread_cond_t       cond_var;         /** Condition variable              */
} _logMqueue_t;


//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

/** The message queue */
static _logMqueue_t * _log_mqueue_ptr = NULL;

/** Thread ID */
static pthread_t _log_thread_id;

/** Flag set when thread is initialized */
static bool _log_thread_is_initialized = false;

/** Flag set when thread shutdown is required */
static bool _thread_is_stopped = false;


//----------------------------------------------------------------------------
//  Local API
//----------------------------------------------------------------------------

/**
 * Initialize message queue
 */
static void _log_mqueue_init()
{
   assert(_log_mqueue_ptr == NULL);

   _log_mqueue_ptr = (_logMqueue_t *) malloc(sizeof(_logMqueue_t));
   assert(_log_mqueue_ptr != NULL);

   memset(_log_mqueue_ptr->messages, 0, _LOG_MAX_MQUEUE_DEPTH * sizeof(_logMessage_t));

   _log_mqueue_ptr->oldest_msg_ptr    = &(_log_mqueue_ptr->messages[0]);
   _log_mqueue_ptr->next_msg_ptr      = &(_log_mqueue_ptr->messages[0]);
   _log_mqueue_ptr->depth             = _LOG_MAX_MQUEUE_DEPTH;
   _log_mqueue_ptr->nb_of_msgs        = 0;

   /* Mutex init */
   if (pthread_mutexattr_init(&(_log_mqueue_ptr->mutex_attr)) != 0)
   {
      perror("pthread_mutexattr_init() failed");
      assert(0 == "pthread_mutexattr_init() failed");
   }

   if (pthread_mutexattr_settype(&(_log_mqueue_ptr->mutex_attr), PTHREAD_MUTEX_RECURSIVE) != 0)
   {
      perror("pthread_mutexattr_settype() failed");
      assert(0 == "pthread_mutexattr_settype() failed");
   }

   if (pthread_mutex_init(
      &(_log_mqueue_ptr->mutex), &(_log_mqueue_ptr->mutex_attr)) != 0)
   {
      perror("pthread_mutex_init() failed");
      assert(0 == "pthread_mutex_init() failed");
   }

   /* Condition variable init */
   if (pthread_condattr_init(&(_log_mqueue_ptr->cond_attr)) != 0)
   {
      perror("pthread_condattr_init() failed");
      assert(0 == "pthread_condattr_init() failed");
   }

   if (pthread_cond_init(
      &(_log_mqueue_ptr->cond_var), &(_log_mqueue_ptr->cond_attr)) != 0)
   {
      perror("pthread_cond_init() failed");
      assert(0 == "pthread_cond_init() failed");
   }
}


/**
 * Destroy message queue
 */
static void _log_mqueue_destroy(void)
{
   if (_log_mqueue_ptr != NULL)
   {
      /* Destroy elements. Don't abort on error */
      if (pthread_condattr_init(&(_log_mqueue_ptr->cond_attr)) != 0)
      {
         perror("pthread_condattr_init() failed");
      }

      if (pthread_mutexattr_destroy(&(_log_mqueue_ptr->mutex_attr)) != 0)
      {
         perror("pthread_mutexattr_destroy() failed");
      }

      /* Free allocated message queue */
      free(_log_mqueue_ptr);
      _log_mqueue_ptr = NULL;
   }
}

/* Lock message queue */
static void _log_mqueue_lock(void)
{
   assert(_log_mqueue_ptr != NULL);

   if (pthread_mutex_lock (&(_log_mqueue_ptr->mutex)) != 0)
   {
      perror("pthread_mutex_lock() failed");
      assert(0 == "pthread_mutex_lock() failed");
   }
}

/* Unlock message queue */
static void _log_mqueue_unlock(void)
{
   assert(_log_mqueue_ptr != NULL);

   if (pthread_mutex_unlock (&(_log_mqueue_ptr->mutex)) != 0)
   {
      perror("pthread_mutex_unlock() failed");
      assert(0 == "pthread_mutex_unlock() failed");
   }
}


/**
 * Push message into message queue
 *
 * @param[in] output    Type of the message output
 * @param[in] msgStr    Pointer on message to be pushed
 * @param[in] param     The parameters (depends on the output type)
 *
 * @return 0 if message was pushed.
 */
static int _log_mqueue_push(logMessageOutput_t output, const char * msgStr, void * param)
{
   int ret = 0;

   assert(_log_mqueue_ptr != NULL);
   assert(msgStr != NULL);

   _log_mqueue_lock();

   if (_log_mqueue_ptr->nb_of_msgs == _log_mqueue_ptr->depth)
   {
      /* Queue is full ! Message is dropped */
      ret = -1;
   }
   else
   {
      if (_log_mqueue_ptr->nb_of_msgs == 0)
      {
         /* Empty queue */
         assert(_log_mqueue_ptr->next_msg_ptr == _log_mqueue_ptr->oldest_msg_ptr);
      }

      /* Prepare new message to be pushed into queue */
      _log_mqueue_ptr->next_msg_ptr->output = output;

      /* Get parameters (copy must be done !!!) */
      switch (output)
      {
      case LOG_MSG_OUT_SYSLOG:
         assert(param != NULL);
         _log_mqueue_ptr->next_msg_ptr->syslog_priority = *((int *)param);
         break;
      case LOG_MSG_OUT_FILE:
         assert(param != NULL);
         _log_mqueue_ptr->next_msg_ptr->file = ((FILE *)param);
         break;
      case LOG_MSG_OUT_DUMMY:
         break;
      default:
         assert(0 == "Invalid message output");
         break;
      }

      strncpy(_log_mqueue_ptr->next_msg_ptr->buffer, msgStr, _LOG_SIZED_FOR(_LOG_MAX_TRACE));
      _log_mqueue_ptr->next_msg_ptr->buffer[_LOG_SIZED_FOR(_LOG_MAX_TRACE) - 1] = 0;


      /* Next message pointer updated */
      if (_log_mqueue_ptr->next_msg_ptr <
         &(_log_mqueue_ptr->messages[_log_mqueue_ptr->depth - 1]))
      {
         ++_log_mqueue_ptr->next_msg_ptr;
      }
      else if (_log_mqueue_ptr->next_msg_ptr ==
         &(_log_mqueue_ptr->messages[_log_mqueue_ptr->depth - 1]))
      {
         /* Circular buffer */
         _log_mqueue_ptr->next_msg_ptr = &(_log_mqueue_ptr->messages[0]);
      }
      else
      {
         assert(0 == "Invalid message pointer (outside array)");
      }

      /* and notify it */
      if (pthread_cond_signal(&(_log_mqueue_ptr->cond_var)) != 0)
      {
         perror("pthread_cond_signal() failed");
         assert(0 == "pthread_cond_signal() failed");
      }

      ++_log_mqueue_ptr->nb_of_msgs;

      ret = 0;
   }

   _log_mqueue_unlock();

   return ret;
}

/**
 * Pop message from queue
 *
 * @param[in] msgPtr Output message buffer
 *
 * @return the received message, NULL if none
 */
static _logMessage_t * _log_mqueue_pop(_logMessage_t *msgPtr)
{
   assert(_log_mqueue_ptr != NULL);
   assert(msgPtr != NULL);

   _log_mqueue_lock();

   if (_log_mqueue_ptr->nb_of_msgs != 0)
   {
      /* Pop the first message */
      msgPtr->output          =  _log_mqueue_ptr->oldest_msg_ptr->output;
      msgPtr->file            =  _log_mqueue_ptr->oldest_msg_ptr->file;
      msgPtr->syslog_priority =  _log_mqueue_ptr->oldest_msg_ptr->syslog_priority;
      strncpy(msgPtr->buffer,
         _log_mqueue_ptr->oldest_msg_ptr->buffer, sizeof(msgPtr->buffer));

      if (_log_mqueue_ptr->oldest_msg_ptr <
         &(_log_mqueue_ptr->messages[_log_mqueue_ptr->depth - 1]))
      {
         ++_log_mqueue_ptr->oldest_msg_ptr;
      }
      else if (_log_mqueue_ptr->oldest_msg_ptr ==
         &(_log_mqueue_ptr->messages[_log_mqueue_ptr->depth - 1]))
      {
         /* Circular buffer */
         _log_mqueue_ptr->oldest_msg_ptr = &(_log_mqueue_ptr->messages[0]);
      }
      else
      {
         assert(0 == "Invalid message pointer (outside array)");
      }

      --_log_mqueue_ptr->nb_of_msgs;
   }
   else
   {
      /* No message in queue */
      msgPtr = NULL;
   }

   _log_mqueue_unlock();

   return msgPtr;
}


/**
 * Wait until message is received and pop it
 *
 * @param[in] msgPtr Output message buffer
 *
 * @return the received message
 */
static _logMessage_t * _log_mqueue_wait_and_pop(_logMessage_t *msgPtr)
{
   assert(_log_mqueue_ptr != NULL);
   assert(msgPtr != NULL);

   _log_mqueue_lock();

   /* Wait until a message is detected in queue */
   while (_log_mqueue_ptr->nb_of_msgs == 0)
   {
      if (pthread_cond_wait(
         &(_log_mqueue_ptr->cond_var), &(_log_mqueue_ptr->mutex)) != 0)
      {
         perror("pthread_cond_wait() failed");
         assert(0 == "pthread_cond_wait() failed");
      }
   }

   /* Pop the first message */
   _log_mqueue_pop(msgPtr);

   _log_mqueue_unlock();

   return msgPtr;
}


/**
 * Log a message
 *
 * @param[in] msgPtr Message buffer
 */
static void _log_thread_log(_logMessage_t *msgPtr)
{
   assert(msgPtr != NULL);

   /* Log message */
   switch (msgPtr->output)
   {
   case LOG_MSG_OUT_SYSLOG:
      syslog( msgPtr->syslog_priority, "%s", msgPtr->buffer );
      break;
   case LOG_MSG_OUT_FILE:
      fprintf(msgPtr->file, "%s\n", msgPtr->buffer);
      break;
   case LOG_MSG_OUT_DUMMY:
      break;
   default:
      assert(0 == "Invalid message output");
      break;
   }
}


/**
 * Flush message queue
 *
 * @param[in] logAllMsg Flag set to true to log all the messages
 */
static void _log_mqueue_flush(bool logAllMsg)
{
   _logMessage_t msgBuffer;

   _log_mqueue_lock();

   /* Wait until a message is detected in queue */
   while (_log_mqueue_ptr->nb_of_msgs != 0)
   {
      /* Pop the first message */
      _log_mqueue_pop(&msgBuffer);

      if (logAllMsg)
      {
         /* Log the message */
         _log_thread_log(&msgBuffer);
      }
   }

   _log_mqueue_unlock();
}


/**
 * The aim of this thread is to create an isolation between application and disk access
 *
 * Rationale: syslog() or write() calls are blocking if disk is flushing,
 * causing unexpected latencies
 */
static void * _log_thread(void * pv)
{
   _logMessage_t msgBuffer;

   while (!_thread_is_stopped)
   {
      /* Wait for a new message to be logged */
      _log_mqueue_wait_and_pop(&msgBuffer);

      /* Don't log message when stop command is received */
      if (!_thread_is_stopped)
      {
         /* Log message */
         _log_thread_log(&msgBuffer);
      }
   }

   return 0;
}


/*
 * Initialize Log thread
 */
static void _log_thread_init(void)
{
   /* Initialized once */
   if (!_log_thread_is_initialized && !_thread_is_stopped)
   {
      /* Initialize message queue */
      _log_mqueue_init();

      /* Create and start thread */
      {
         pthread_attr_t attrs;

         if (pthread_attr_init(&attrs) != 0)
         {
            perror("pthread_attr_init() failed");
            assert(0 == "pthread_attr_init() failed");
         }

         if ( pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE) != 0)
         {
            perror("pthread_attr_setdetachstate() failed");
            assert(0 == "pthread_attr_setdetachstate() failed");
         }

         if (pthread_create(&_log_thread_id, &attrs, _log_thread, NULL) != 0)
         {
            perror("pthread_create() failed");
            assert(0 == "pthread_create() failed");
         }
      }

      /* Log thread initialized */
      _log_thread_is_initialized = true;
   }
}

/*
 * Exiting Log thread
 */
static void _log_thread_stop(void)
{
   /* Stop once */
   if (_log_thread_is_initialized && !_thread_is_stopped)
   {
      /* Stop command sent: stop flag set and dummy message pushed in order to exit immediately */
      _thread_is_stopped = true; /* Stop command set */
      _log_mqueue_push(LOG_MSG_OUT_DUMMY, "STOP", NULL);
      pthread_join(_log_thread_id, NULL);

      /* Destroy queue */
      _log_mqueue_destroy();

      /* Log thread no more initialized */
      _log_thread_is_initialized = false;
   }
}

/**
 * Push log message into queue for logging
 *
 * @param[in] output    The message output
 * @param[in] msgStr    Pointer on message to be logged
 * @param[in] param     Custom parameters. Depends on the output
 *
 * @return 0 if message was pushed into queue
 */
static int _push_log_to_queue(logMessageOutput_t output, const char * msgStr, void * param)
{
   int ret = 0;
   static size_t nb_dropped_msg = 0;
   static char buffer[50] = {0};
   int priority;
   void * errorParam = NULL;

   assert(msgStr != NULL);

   _log_lock();

   ret = 1;

   /* Initialize thread on first message to be logged */
   if (_log_thread_is_initialized == false)
   {
      _log_thread_init();

      assert(_log_thread_is_initialized == true);
   }

   if (nb_dropped_msg > 0)
   {
      /* Warn about dropped logs */
      snprintf ( buffer, sizeof(buffer), "##### Dropped logs: %lu #####", nb_dropped_msg );

      switch (output)
      {
      case LOG_MSG_OUT_SYSLOG:
         priority = LOG_ERR;
         errorParam = (void *)&priority;
         break;
      case LOG_MSG_OUT_FILE:
         errorParam = param; /* Output is a file: keep same parameters */
         break;
      default:
         assert(0 == "Invalid message output");
         break;
      }

      if (_log_mqueue_push(output, buffer, errorParam) == 0)
      {
         /* Erase counter */
         nb_dropped_msg = 0;

         /* Push message */
         if (_log_mqueue_push(output, msgStr, param) != 0 )
         {
            /* Message cannot be pushed: dropped */
            ++nb_dropped_msg;
         }
         else
         {
            ret = 0;
         }
      }
   }
   else
   {
      /* Push message directly */
      if (_log_mqueue_push(output, msgStr, param) != 0 )
      {
         /* Message cannot be pushed: dropped */
         ++nb_dropped_msg;
      }
      else
      {
         ret = 0;
      }
   }

   _log_unlock();

   return ret;
}

//----------------------------------------------------------------------------
//  Public API
//----------------------------------------------------------------------------

/**
 * Push log message into thread queue for syslog logging
 *
 * @param[in] string    Pointer on message to be logged
 * @param[in] priority  Syslog priority
 *
 * @return 0 if message was pushed into queue
 */
int push_syslog_log_to_queue(const char * string, int priority)
{
   return _push_log_to_queue(LOG_MSG_OUT_SYSLOG, string, (void *)&priority);
}

/**
 * Push log message into thread queue for file logging
 *
 * @param[in] string    Pointer on message to be logged
 * @param[in] file      Output file
 *
 * @return 0 if message was pushed into queue
 */
int push_file_log_to_queue(const char * string, FILE * file)
{
   return _push_log_to_queue(LOG_MSG_OUT_FILE, string, (void *)(file));
}

/**
 * Stop log thread
 */
void stop_log_thread()
{
   _log_lock();

   if (_log_thread_is_initialized == true)
   {
      /* Stop thread */
      _log_thread_stop();
   }

   _log_unlock();
}

/**
 * Flush the logs contained in the queue
 *
 * @param[in] logAllMsg Flag set to log messages
 */
void flush_log_queue(bool logAllMsg)
{
   _log_lock();

   _log_mqueue_flush(logAllMsg);

   _log_unlock();
}

#endif /* ndef LOG_THREADED_LOG */

/* ---------------------------- End of file ------------------------------- */
