/*-
 *****************************************************************************
 * Defines all the limits used by the logger component
 *
 * @author gpa
 *****************************************************************************
 */


//----------------------------------------------------------------------------
//  Local defines and macros
//----------------------------------------------------------------------------

#ifndef LOGGER_SMALL

/** Start of log split */
#define _LOG_SPLIT_PATTERN \
   "\n##########################################################################\n\n"

/** Size of the trace buffer in which the debug statement is formed */
#define _LOG_MAX_TRACE                 1024

/** Size of the buffer holding the string representation of the function */
#define _LOG_MAX_FUNCTION_REPR_LENGTH   64

/** Size of the buffer holding the string representation of the file name */
#define _LOG_MAX_FILE_REPR_LENGTH       48

/** Maximum length for the process name */
#define _LOG_MAX_PROCESS_NAME_LENGTH    16

/** Maximum chars in thread id */
#define _LOG_MAX_THREAD_ID_REPR_LENGTH   12

/** Maximum chars in line number representation */
#define _LOG_MAX_LINE_NUMBER_REPR_LENGTH 8

/** Maximum length of domains names */
#define _LOG_MAX_DOMAIN_REPR_LENGTH     32

/** Maximum number of domains supported */
#define _LOG_MAX_DOMAINS                32

/** Define the maximum number of supported domain level filters */
#define _LOG_MAX_DOMAIN_LEVEL_FILTERS   16

/** Define the maximum number of supported domain file redirection */
#define _LOG_MAX_DOMAIN_FILE_REDIRECTION   16

/** Maximum length of the command line */
#define _LOG_MAX_COMMAND_LINE_LENGTH    256

/* Depth of the Message Queue (thread mode) */
#define _LOG_MAX_MQUEUE_DEPTH           250

#else /* def LOGGER_SMALL */

/** Start of log split */
#define _LOG_SPLIT_PATTERN \
   "\n##########################################################################\n\n"

/** Size of the trace buffer in which the debug statement is formed */
#define _LOG_MAX_TRACE                  128

/** Size of the buffer holding the string representation of the function */
#define _LOG_MAX_FUNCTION_REPR_LENGTH   32

/** Size of the buffer holding the string representation of the file name */
#define _LOG_MAX_FILE_REPR_LENGTH       16

/** Maximum length for the process name */
#define _LOG_MAX_PROCESS_NAME_LENGTH    4

/** Maximum chars in thread id */
#define _LOG_MAX_THREAD_ID_REPR_LENGTH  8

/** Maximum chars in line number representation */
#define _LOG_MAX_LINE_NUMBER_REPR_LENGTH 5

/** Maximum length of domains names */
#define _LOG_MAX_DOMAIN_REPR_LENGTH     8

/** Maximum number of domains supported */
#define _LOG_MAX_DOMAINS                8

/** Define the maximum number of supported domain level filters */
#define _LOG_MAX_DOMAIN_LEVEL_FILTERS   4

/** Define the maximum number of supported domain file redirection */
#define _LOG_MAX_DOMAIN_FILE_REDIRECTION  0

/** Maximum length of the command line */
#define _LOG_MAX_COMMAND_LINE_LENGTH    32

/* Depth of the Message Queue (thread mode) */
#define _LOG_MAX_MQUEUE_DEPTH           20

#endif

/**
 * Helper macro allocates enough space for the number of represented character
 * and the null terminating char.
 */
#define _LOG_SIZED_FOR(repr_length) (repr_length+1)

/** Define the maximum length of domains string */
#define _LOG_MAX_DOMAIN_LIST_LENGTH     (_LOG_MAX_DOMAINS * (_LOG_MAX_DOMAIN_REPR_LENGTH + 1))


/* ---------------------------- End of file ------------------------------- */
