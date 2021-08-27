#ifndef logger_os_included_H
#define logger_os_included_H
/*-
 *****************************************************************************
 * Abstract API and type for OS specific tasks and locks
 * This is a private header part of the log library.
 *
 * @author gpa
 *****************************************************************************
 */

#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{  //  Begining of extern "C" for all functions
#endif

//----------------------------------------------------------------------------
//  Helper METHODS scoped to OS implementations only
//----------------------------------------------------------------------------
const char *_log_get_process_name();


//----------------------------------------------------------------------------
//  Helper TYPES scoped to OS implementations only
//----------------------------------------------------------------------------

/** Abstract thread id which can be printed with a %d */
typedef unsigned long _LOG_THREAD_ID;


//----------------------------------------------------------------------------
//  OVERRIDES - Each OS must provide an implementation for each of these
//----------------------------------------------------------------------------

/** Get hold of the current thread id */
_LOG_THREAD_ID _log_get_current_thread_id();

/** Copy the thread ID to the output buffer. Allow getting the thread's name */
void _log_copy_thread_id( char *buffer, size_t count, _LOG_THREAD_ID id);

/** Get hold of the configuration string */
const char *log_get_config_string();

/** Get hold of the timestamp as a string. If epoch is non 0, use the supply date. */
const char *log_os_timestamp(unsigned long epoch);

/** Parse extra config arguments */
bool log_os_parse_config( char *config);

/** Get the process name and store in global */
void log_os_set_process_name(char *_log_process_name, size_t max);

/** Logger exit function */
void _log_exit(void);

/** Print function - the text is already formatted */
void _log_print(const char *string);

/** Create a portable mutex */
void _log_mutex_init();

/** Lock log mutex which protects printf operations */
void _log_lock();

/** Unlock log mutex */
void _log_unlock();


#ifdef __cplusplus
}
#endif


#endif /* ndef logger_os_included_H */
