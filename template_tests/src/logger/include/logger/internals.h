#ifndef __LOGGER_INTERNALS_INCLUDED_H__
#define __LOGGER_INTERNALS_INCLUDED_H__
/*-
 ****************************************************************************
 * Helping library for tracing
 * Please check tracing workbook
 *
 * @author      ste, re-written gpa, mrl, gpa
 ****************************************************************************
 */


/*****************************************************************************
 *  Configuration
 ****************************************************************************/
#ifndef _ASMLANGUAGE       /* C types are not defined for assembler compile */
#ifndef __LOGGER_INCLUDED_H__
#  error "This file cannot be included directly. Use log.h instead"
#endif


/*****************************************************************************
 *****************************************************************************
 *
 *  DEBUG case
 *
 *****************************************************************************
 ****************************************************************************/
#ifndef FORCE_NODEBUG


/*****************************************************************************
 *  Includes - Only when debugging
 ****************************************************************************/
#ifdef __cplusplus
#  include <cstdlib>
#  include <cstdio>
#  include <sstream>
#  include <string>
#else
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdbool.h>
#endif /* def __cplusplus */


/*
 *  All functions and types are declared as 'C' entities to allow C programs
 *   to use this library.
 */
#ifdef __cplusplus

#ifdef __GNUC__
#  include <typeinfo>
#endif
#if defined _MSC_VER
#  include <typeinfo.h>
#endif


/**
 *****************************************************************************
 * Helper class which is used to add IN/OUT statement within methods or
 *  functions by taking advantage of the C++ construction/destruction strategy
 *  of objects created on the stack
 *****************************************************************************
 */
class _logScopeObj_c
   {
public:
   _logScopeObj_c(
      unsigned long line,
      const char   *file,
#ifdef __GNUC__
      const char   *function,
#endif
      const void   *pThis,
      const char   *classname,
      const char   *mask );
   ~_logScopeObj_c();
protected:
   void out( const char *strInOrOut );
private:
   unsigned long m_line;
   const char   *m_file;
#ifdef __GNUC__
   const char   *m_function;
#endif
   const void   *m_this;
   const char   *m_classname;
   const char   *m_mask;
   };

   // Recall C method to avoid the compiler from implicitly casting
   //  the trace back to the C++ prototype, causing a endless loop (and crashing the code)
   extern "C" void _log_trace( const char *mask, const char *format, ...);
   extern "C" void _log_trace_error( const char *mask, const char *str );

   inline void _log_trace( const char *mask, const std::string& str)
      { _log_trace( mask, "%s", str.c_str()); }

extern "C" {
#endif // def __cplusplus


/*****************************************************************************
 *  Exported API
 ****************************************************************************/
#ifdef LOGGER_SMALL
void _log_init( bool splitPatternOff, const char *log_env_to_copy );
#else
void _log_init( bool splitPatternOff );
#endif
#ifdef __GNUC__
/* The GNU compiler can check that the arguments are a match for the formatter */
void _log_trace( const char *mask, const char *format, ...) __attribute__ ((format(printf, 2, 3)));
void _log_printf( const char *format, ...) __attribute__ ((format(printf, 1, 2)));
#else
void _log_trace( const char *mask, const char *format, ...);
void _log_printf( const char *format, ...);
#endif
void _log_trace_error( const char *mask, const char *str );
void _log_lock( void );  // Lock the log mutex to prevent smearing
void _log_set_mask( const char *mask );
void _log_set_not_mask( const char *mask );
void _log_get_masks( char *masks, size_t lMasks, char *notMasks, size_t lNotMasks );
void _log_get_not_mask( char *maskString, size_t size );
void _log_set_level( logLevel_t Level );
logLevel_t _log_get_level( );
void _log_dump( const char *mask, void *pStart, long howMany );
int  _log_set_domain_level( const char *domains, logLevel_t level );
int  _log_clear_domain_level( const char *domains );
int  _log_get_domain_levels( logDomainLevelPair_t* map, size_t maxBuffer, size_t maxDomains );
int  _log_set_domain_redirection( const char *domains, FILE *pf );
long _log_get_limit( logLimit_t limitType );
bool _log_filter_trace( const char *domain, logLevel_t logLevel );
void _log_trace_stack();
void _log_set_datetime_callback( LogDatetime_t );
void _log_set_external_logger( LogCallback_t );


/*****************************************************************************
 *  Exported Data
 ****************************************************************************/
extern logLevel_t     _log_level;      /* Current user level     */
extern logLevel_t     _log_type;       /* Level requested        */
extern unsigned long  _log_line;       /* Line when invoking     */
extern const char    *_log_filename;   /* Filename when invoking */

#ifdef __GNUC__
extern const char    *_log_function;   /* Function name          */
#endif

extern FILE   *_log_outfile;           /* Output file or 0 for system logger */
extern char  **_log_public_domain_level_lookup;  /* Fake domain level lookup */

#ifdef LOGGER_SMALL
#  define _LOG_INIT(x)                  _log_init(false, x)
#else
#  define _LOG_INIT()                   _log_init(false)
#  define _LOG_INIT_NO_SPLIT_PATTERN()  _log_init(true)
#endif
#  define _LOG_MASK(mask)               _log_set_mask(mask)
#  define _LOG_NOTMASK(mask)            _log_set_not_mask(mask)
#  define _LOG_GETMASKS(a,b,c,d)        _log_get_masks(a,b,c,d);
#  define _LOG_SETLEVEL(level)          _log_set_level((logLevel_t)level)
#  define _LOG_GETLEVEL()               _log_get_level()
#  define _LOG_GETLIMIT(limit)          _log_get_limit(limit)
#  define _LOG_SETDOMAINLEVEL(d,l)      _log_set_domain_level(d, (logLevel_t)l)
#  define _LOG_CLEARDOMAINLEVEL(d)      _log_clear_domain_level(d)
#  define _LOG_GETDOMAINLEVELS(a,b,c)   _log_get_domain_levels(a,b,c)
#  define _LOG_SETDOMAINREDIRECTION(d,f) _log_set_domain_redirection(d,f)
#  define _LOG_TRACE_STACK()            _log_trace_stack();
#  define _LOG_SET_DATETIME_CALLBACK(c) _log_set_datetime_callback(c)
#  define _LOG_SET_EXTERNAL_LOGGER(c)   _log_set_external_logger(c)
#  define _LOG_PRINT                    _log_printf

#  define _LOG_EMPTY_IF_NULL(s)         (s==0?"empty":s)
#  define _LOG_ONLY(exp)                exp

#ifdef __GNUC__
#  define _LOG_TRACE_AT_LEVEL(level) \
      _log_lock(), \
      _log_line     = __LINE__,     \
      _log_filename = __FILE__,     \
      _log_function = __FUNCTION__, \
      _log_type     = level, \
      _log_trace
#endif // def __GNUC__

#  define _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY \
    (*((char *)_log_public_domain_level_lookup)==0)

#  define _LOG_CHECK_LEVEL_AND_TRACE(level) \
      if ( _log_level < level && _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ) {} else _LOG_TRACE_AT_LEVEL(level)

#  define _LOG_CHECK(dom,level) \
      ( ( (_log_level >= level) || !_LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY) && _log_filter_trace(dom,level) )

#  define _LOG_ERROR _LOG_TRACE_AT_LEVEL(LOG_LEVEL_ERROR)
#  define _LOG_WARN  _LOG_CHECK_LEVEL_AND_TRACE(LOG_LEVEL_WARN)
#  define _LOG_MILE  _LOG_CHECK_LEVEL_AND_TRACE(LOG_LEVEL_MILE)
#  define _LOG_INFO  _LOG_CHECK_LEVEL_AND_TRACE(LOG_LEVEL_INFO)
#  define _LOG_TRACE _LOG_CHECK_LEVEL_AND_TRACE(LOG_LEVEL_TRACE)
#  define _LOG_DEBUG _LOG_CHECK_LEVEL_AND_TRACE(LOG_LEVEL_DEBUG)

#  ifdef __GNUC__

/* Special case for assert - we do not want to strip the code, by simply to error */
#     define _LOG_ASSERT(exp, expstr) \
      { \
         if ( ! (exp) ) { \
            _LOG_ERROR( NULL, "Assertion %s failed", expstr); \
            _log_trace_stack(); \
            abort(); \
         } \
      } \

#     ifdef __cplusplus

         #ifdef __GNUC__
            #define _LOG_STREAM(dom,level,os) \
               if ( _log_level < level && _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ) {} \
               else { \
                  _log_lock(); \
                  _log_type = level; \
                  _log_line = __LINE__; \
                  _log_filename = __FILE__; \
                  _log_function = __FUNCTION__; \
                  std::ostringstream ss; ss << os; \
                  _log_trace(dom, "%s", ss.str().c_str()); }
         #else
            #define _LOG_STREAM(dom,level,os) \
               if ( _log_level < level && _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY ) {} \
               else { \
                  _log_lock(); \
                  _log_type = level; \
                  _log_line = __LINE__; \
                  _log_filename = __FILE__; \
                  std::ostringstream ss; ss << os; \
                  _log_trace(dom, "%s", ss.str().c_str()); }
         #endif
#        define _LOG_THIS_HEADER(dom) \
            volatile _logScopeObj_c _log_scope = _logScopeObj_c( \
               __LINE__, __FILE__, __FUNCTION__, this, typeid( *this ).name(), dom )
#        define _LOG_HEADER(dom) \
            volatile _logScopeObj_c _log_scope( __LINE__, __FILE__, __FUNCTION__, 0, 0, dom )
#    endif
#  else  /*ndef GNUC */
#     define _LOG_TRACE_AT_LEVEL(level) \
         _log_lock(), \
         _log_line     = __LINE__,     \
         _log_filename = __FILE__,     \
         _log_type     = level, \
         _log_trace

#     ifdef __cplusplus
#        define _LOG_THIS_HEADER(dom) \
            volatile _logScopeObj_c _log_scope = _logScopeObj_c( \
               __LINE__, __FILE__, this, typeid( *this ).name(), dom )
#        define _LOG_HEADER(dom) \
                        volatile _logScopeObj_c _log_scope( __LINE__, __FILE__, 0, 0, dom )
#    endif
#  endif

#ifdef __cplusplus
}
#endif


/*****************************************************************************
 *****************************************************************************
 *
 *  NDEBUG case : All macros have no effect
 *
 *****************************************************************************
 ****************************************************************************/
#else /* FORCE_NODEBUG */

/*
 *  These functions are never defined. They are here to satisfy the compiler
 *   when it sees the TRACE, WARN and LOG definitions below.
 */
#ifdef __cplusplus
extern "C"
#endif

#  define _LOG_INIT(...)                ((void)0)
#  define _LOG_MASK(a)                  ((void)0)
#  define _LOG_NOTMASK(a)               ((void)0)
#  define _LOG_GETMASKS(a,b,c,d)        ((void)0)
#  define _LOG_SETLEVEL(a)              ((void)0)
#  define _LOG_GETLEVEL()               ((int)-1)
#  define _LOG_GETLIMIT(a)              ((int)-1)
#  define _LOG_SETDOMAINLEVEL(a,b)      ((void)0)
#  define _LOG_GETDOMAINLEVELS(a,b,c)   ((int)-1)
#  define _LOG_CLEARDOMAINLEVEL(d)      ((void)0)
#  define _LOG_SETDOMAINREDIRECTION(d,f) ((void)0)
#  define _LOG_TRACE_STACK()            ((void)0)
#  define _LOG_SET_DATETIME_CALLBACK(c) ((void)0)
#  define _LOG_SET_EXTERNAL_LOGGER(c)   ((void)0)
#  define _LOG_PRINT(...)               ((void)0)

#  define _LOG_ERROR(...)               ((void)0)
#  define _LOG_WARN(...)                ((void)0)
#  define _LOG_MILE(...)                ((void)0)
#  define _LOG_INFO(...)                ((void)0)
#  define _LOG_TRACE(...)               ((void)0)
#  define _LOG_DEBUG(...)               ((void)0)
#  define _LOG_ASSERT(exp, s)           { (void)s; (void)(exp); }
#  define _LOG_ONLY(exp)

#  define _LOG_HEADER(...)              ((void)0)
#  define _LOG_THIS_HEADER(...)         ((void)0)

#endif /* _DEBUG */

#endif /* ndef _ASMLANGUAGE */
#endif /* ndef __LOGGER_INTERNALS_INCLUDED_H__ */
