#ifndef __LOGGER_INCLUDED_H__
#define __LOGGER_INCLUDED_H__
/*-
 *****************************************************************************
 * Tracing library public MACROS
 * In order to use the debugging facilities of this library,
 *  simply include this file.
 * It is recommended that this file should be included AFTER the
 *  standard system libraries in order to avoid having some of
 *  its macros overridden.
 *
 * @usage       Define NDEBUG to disable this library
 *
 * @author      ste, mrl, gpa
 *****************************************************************************
 */


/*----------------------------------------------------------------------------
 *  Public types
 *--------------------------------------------------------------------------*/

/** Tracing levels */
typedef enum
{
   /** For internal errors or serious conditions */
   LOG_LEVEL_ERROR=0,
   /** For managed error conditions */
   LOG_LEVEL_WARN,
   /** For system milestones */
   LOG_LEVEL_MILE,
   /** For important internal info */
   LOG_LEVEL_INFO,
   /** For detailing internal functions */
   LOG_LEVEL_TRACE,
   /** For very detailed tracing */
   LOG_LEVEL_DEBUG,
} logLevel_t;


/** Pair specifying a domain debug level */
typedef struct
{
   /** Points to a character buffer receiving the domain name */
   char      *domain;
   /** Specify the level for a domain */
   logLevel_t level;
} logDomainLevelPair_t;

/** Type of callback to intercept all trace calls externally */
typedef void (*LogCallback_t)(
   int,             // Line number
   const char *,    // File name
   const char *,    // Function name
   unsigned long,   // Thread id
   logLevel_t,      // Level of the statement
   const char *,    // Domain
   const char *     // String
);

/** Type of the callback for getting the clock */
typedef unsigned long (*LogDatetime_t)(void);

/** Limits */
typedef enum {
   /** Internal buffer size */
   LOG_LIMIT_TRACE,
   /** Maximum length of domains names */
   LOG_LIMIT_DOMAIN_REPR,
   /** Max length of the function name */
   LOG_LIMIT_FUNCTION_REPR,
   /** Max length of the file name */
   LOG_LIMIT_FILE_REPR,
   /** Max length for the process name */
   LOG_LIMIT_PROCESS_REPR,
   /** Maximum length of the thread id representation */
   LOG_LIMIT_THREAD_ID_REPR,
   /** Maximum chars in line number representation */
   LOG_LIMIT_LINE_REPR,
   /** Maximum number of supported domain level filters */
   LOG_LIMIT_DOMAIN_LEVEL_FILTERS,
   /** Maximum number of threads which can to banned */
   LOG_LIMIT_BANNED_THREADS,
   /** Maximum number of domains supported */
   LOG_LIMIT_DOMAINS,
   /** Maximum possible length for the domain mask */
   LOG_LIMIT_MASKS,
} logLimit_t;

/*----------------------------------------------------------------------------
 *  Includes
 *--------------------------------------------------------------------------*/
#include "logger/internals.h"


/*----------------------------------------------------------------------------
 *  Public macros - all tracing functions are provided as macros to allow
 *   a full strip for memory sensitive targets
 *--------------------------------------------------------------------------*/

/** @see _log_init */
#  undef  LOG_INIT
#  define LOG_INIT                     _LOG_INIT
/** @see _log_init */
#  undef  LOG_INIT_NO_SPLIT_PATTERN
#  define LOG_INIT_NO_SPLIT_PATTERN    _LOG_INIT_NO_SPLIT_PATTERN
/** @see _log_mask */
#  undef  LOG_MASK
#  define LOG_MASK(mask)               _LOG_MASK(mask)
/** @see _log_not_mask */
#  undef  LOG_NOTMASK
#  define LOG_NOTMASK(mask)            _LOG_NOTMASK(mask)
/** @see _log_get_masks */
#  undef  LOG_GETMASKS
#  define LOG_GETMASKS                 _LOG_GETMASKS
/** @see _log_set_level */
#  undef  LOG_SETLEVEL
#  define LOG_SETLEVEL(level)          _LOG_SETLEVEL(level)
/** @see _log_get_level */
#  undef  LOG_GETLEVEL
#  define LOG_GETLEVEL()               _LOG_GETLEVEL()
/** @see _log_get_domain_levels */
#  undef  LOG_GETDOMAINLEVELS
#  define LOG_GETDOMAINLEVELS(a,b,c)   _LOG_GETDOMAINLEVELS(a,b,c)
/** @see _log_set_domain_level */
#  undef  LOG_SETDOMAINLEVEL
#  define LOG_SETDOMAINLEVEL(d,l)      _LOG_SETDOMAINLEVEL(d,l)
/** @see _log_clear_domain_level */
#  define LOG_CLEARDOMAINLEVEL(d)      _LOG_CLEARDOMAINLEVEL(d)
/** @see _log_set_domain_redirection */
#  undef  LOG_SETDOMAINREDIRECTION
#  define LOG_SETDOMAINREDIRECTION(d,f) _LOG_SETDOMAINREDIRECTION(d,f)
/** @see _log_get_limit */
#  undef  LOG_GETLIMIT
#  define LOG_GETLIMIT                 _LOG_GETLIMIT
/** @see _log_filter_trace */
#  undef  LOG_CHECK
#  define LOG_CHECK(d,l)               _LOG_CHECK(d,l)

/** Use to protect strings that may be null */
#  undef  LOG_EMPTY_IF_NULL
#  define LOG_EMPTY_IF_NULL(s)         _LOG_EMPTY_IF_NULL(s)

// Intrusive  methods

/**
 * Allow an application to provide the date time
 * The prototype of the callback is long ();
 */
#  undef  LOG_SET_DATETIME_CALLBACK
#  define LOG_SET_DATETIME_CALLBACK    _LOG_SET_DATETIME_CALLBACK

/**
 * Allow an external application to collect debug statements
 */
#  undef  LOG_SET_EXTERNAL_LOGGER
#  define LOG_SET_EXTERNAL_LOGGER      _LOG_SET_EXTERNAL_LOGGER

#ifdef __cplusplus
/** C++ only. Use inside a non-static member function to automatically mark the start and end of the method */
#  undef  LOG_THIS_HEADER
#  define LOG_THIS_HEADER              _LOG_THIS_HEADER
/** C++ only. Use inside a method to automatically mark the start and end of the method */
#  undef  LOG_HEADER
#  define LOG_HEADER                   _LOG_HEADER
/** C++ only. Allow to use ostream constructs */
#  undef  LOG_STREAM
#  define LOG_STREAM _LOG_STREAM
#endif


/* Undefine these popular macros by precaution */
#  undef  LOG_ERROR
#  undef  LOG_WARN
#  undef  LOG_MILE
#  undef  LOG_INFO
#  undef  LOG_TRACE
#  undef  LOG_DEBUG
#  undef  LOG_ASSERT
#  undef  LOG_PRINT

/** For internal errors or serious conditions */
#  define LOG_ERROR              _LOG_ERROR
/** For managed error conditions */
#  define LOG_WARN               _LOG_WARN
/** For system milestones */
#  define LOG_MILE               _LOG_MILE
/** For detailing internal functions */
#  define LOG_INFO               _LOG_INFO
/** For detailing internal functions */
#  define LOG_TRACE              _LOG_TRACE
/** For detailed tracing */
#  define LOG_DEBUG              _LOG_DEBUG
/** For printing through the logger channel */
#  define LOG_PRINT              _LOG_PRINT
/** @see log_assert */
#ifndef POLYSPACE
#  define LOG_ASSERT(expression) _LOG_ASSERT(expression, #expression)
#else
#  define LOG_ASSERT(expression) assert(expression)
#endif
/** Use this to enclose a block of statements to be stripped in release builds on some targets */
#  define LOG_ONLY(expression)   _LOG_ONLY(expression)


/**
 * Attempts to prints the execution stack. This can be useful for targets where
 *  a core cannot be produced or to add the stack trace directly into the logger.
 * On some targets, this will do nothing
 * The best way to use is to override the terminate and log the stack.
 * This will show the exception trail.
 * <pre>
 * #include <exception>
 * void myterminate () { LOG_TRACE_STACK(); abort(); }
 * int main() { std::set_terminate (myterminate); ... }
 * </pre>
 */
#  define LOG_TRACE_STACK        _LOG_TRACE_STACK

#endif /* ndef __LOGGER_INCLUDED_H__ */
