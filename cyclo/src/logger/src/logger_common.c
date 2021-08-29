/*-
 *****************************************************************************
 * Implementation of some of the program logging/tracing functionality to help
 * debugging.
 * This is the common base for all OSes
 *
 * @author gax
 *****************************************************************************
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

//----------------------------------------------------------------------------
//  Local include
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "logger.h"
#include "logger/logger_os.h"
#include "logger/logger_limits.h"


/**
 * Helper macro allocates enough space for the number of represented character
 * and the null terminating char.
 */
#define _LOG_SIZED_FOR(repr_length) (repr_length+1)

#ifdef _MSC_VER
// Note : This API is part of the C99 and C11 standard but MS do not support them!
int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

int snprintf(char* str, size_t size, const char* format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = vsnprintf(str, size, format, ap);
	va_end(ap);

	return count;
}
#endif


//----------------------------------------------------------------------------
//  Local types
//----------------------------------------------------------------------------

/**
 *  Memorizes previous states to decide whether to dump
 *   some if the information again or not.
 */
typedef struct {
   _LOG_THREAD_ID thread_id;
   const char    *filename;
   const char    *function;
   const char    *domain;
} _logSetting_t;


/** Defines a domain string identifier */
typedef struct
{
   char name[_LOG_MAX_DOMAIN_REPR_LENGTH];
} _logDomainIdentifier_t;


/** Define the per domain debug levels */
typedef struct
{
   _logDomainIdentifier_t domain;
   logLevel_t level;
} _logDomainLevelPair_t;

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
/** Define the per domain file redirection */
typedef struct
{
   _logDomainIdentifier_t domain;
   FILE * pf;
} _logDomainFilePair_t;
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

// These variables are set in the various trace macro.
// They are protected by a mutex.

/** Latest file name stored here */
const char    *_log_filename;

/** Latest line number stored here */
unsigned long  _log_line;

/** The debug level requested at the time by one of the macros */
logLevel_t    _log_type;

/** Latest function name stored here */
const char    *_log_function;

/**
 * The Level is the current level of debugging.
 * This variable is tested within the various trace macros
 */
logLevel_t   _log_level = LOG_LEVEL_MILE;

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
/** File pointer where the tracing will go */
FILE   *_log_outfile = 0;

/** Allow specific file redirection on specific domains */
_logDomainFilePair_t _log_domain_file_lookup[_LOG_MAX_DOMAIN_FILE_REDIRECTION];
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

/** Allow specific levels on specific domains */
_logDomainLevelPair_t _log_domain_level_lookup[_LOG_MAX_DOMAIN_LEVEL_FILTERS];

/** Set the fake domain level lookup */
char **_log_public_domain_level_lookup = (char **)_log_domain_level_lookup;

/**
 *  The mask represent which domains should be displayed
 *  This list contains the valid domain at run-time
 *  If empty, all domains are accepted
 */
static _logDomainIdentifier_t _log_mask[_LOG_MAX_DOMAINS + 1 /* End Mark */];

/** @see _log_mask */
static _logDomainIdentifier_t _log_not_mask[_LOG_MAX_DOMAINS + 1 /* End Mark */];

/** Structure with settings since the last call to trace */
static _logSetting_t _log_last;

/** Local static buffer for forming strings prior to displaying them.
 *  This buffer should prevent alloc / dealloc which is time costly
 *   and can lead to fragmentation.
 */
static char _log_buffer[_LOG_SIZED_FOR(_LOG_MAX_TRACE)] = {0};

/** Place holder for the process name */
static char _log_process_name[_LOG_SIZED_FOR(_LOG_MAX_PROCESS_NAME_LENGTH)] = {0};

/** Flag to mark whether the output full and unindented */
static bool _log_full = false;

/** External function that gives the date time */
static LogDatetime_t _log_datetime_callback = NULL;

/** Callback for external logger */
static LogCallback_t _log_logger_callback = NULL;

/** Separators between domains */
static const char seps[]   = "|:;,!/";

/** Elipsis pattern for truncation */
static const char elipsis[] = "...";


//----------------------------------------------------------------------------
//  Public API
//----------------------------------------------------------------------------
/** Reset domain level filtering */
void _log_reset_domain_level_filters()
   { memset(_log_domain_level_lookup, 0, sizeof(_log_domain_level_lookup)); }

/** Return the process name. Valid only shortly after initialisation */
const char *_log_get_process_name()
   { return _log_process_name; }

void _log_reset_last_function(void)
   { _log_last.function = 0; }

/**
 *****************************************************************************
 * Replacement function for strtok_r which is non ANSI
 *
 * @see Man page of strtok_r
 * @author gpa
 *****************************************************************************
 */
static char *log_strtok_r(char *s, const char *delim, char **last)
{
   char *spanp;  int c, sc;  char *tok;

   if (s == 0 && (s = *last) == 0)
   {
      return 0;
   }

cont:
   c = *s++;

   for (spanp = (char *)delim; (sc = *spanp++) != 0; )
   {
      if (c == sc)
      {
         goto cont;
      }
   }

   if (c == 0)
   {
      *last = 0;
      return 0;
   }

   tok = s - 1;

   for (;;)
   {
      c = *s++;
      spanp = (char *)delim;

      do
      {
         if ((sc = *spanp++) == c)
         {
            if (c == 0)
            {
               s = 0;
            }
            else
            {
               char *w = s - 1;
               *w = '\0';
            }

            *last = s;

            return tok;
         }
      } while (sc != 0);
   }
}

/**
 * Replacement for strncpy which accepts array and always terminate the string.
 * The function copies as much as possible and terminates with a zero.
 * @param dst Destination string
 * @param src Source string to copy
 * @param size Maximum number of char to copy including the terminating zero
 * @return The number of char that could have been copied, excluding the zero
 */
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = 0;

    while ( size > 1 && *src )
    {
       *dst++ = *src++;
       --size, ++len;
    }

    if ( size > 0 )
    {
        *dst = 0;
    }

    return len + strlen(src);
}


/**
 * Compare against a wildcard string.
 * The wildcard char is '*'. However, only 1 single wildcard is supported.
 * The a* will match a, ab a.b
 * @param wild A string to compare against which could have wildcards
 * @param str String to match against a wildcard expression
 * @param bestScore Score returned by a previous match allowing comparing multiple matches
 *        For the first call, set to 0.
 * @return true if there is a match and a best score
 */
bool wildcmp(const char *wild, const char *str, int *bestScore)
{
   const char *cp = NULL, *mp = NULL;
   int newScore = 0;

   while ((*str) && (*wild != '*'))
   {
      if ((*wild != *str) && (*wild != '?'))
      {
         return false;
      }

      if ( *wild != '?' )
      {
         ++newScore;
      }

      ++wild;
      ++str;
   }

   while (*str)
   {
      if (*wild == '*')
      {
         if (!*++wild)
         {
            break;
         }

         mp = wild;
         cp = str+1;
      }
      else if ( *wild == '?' )
      {
         ++wild;
         ++str;
      }
      else if (*wild == *str)
      {
         ++wild;
         ++str;
         ++newScore;
      }
      else
      {
         wild = mp;
         str = cp++;
      }
   }

   while (*wild == '*')
   {
      ++wild;
   }

   if ( (!*wild) && (*bestScore<=0 || newScore > *bestScore ) )
   {
      *bestScore=newScore;
      return true;
   }

   return false;
}


/**
 *****************************************************************************
 * Skip all blanks, including space, tabs and control chars.
 *
 * @param p  A pointer to the start of the string
 * @return  A pointer to the first non-blank character
 * @author gpa
 *****************************************************************************
 */
static char *log_skip_blanks( char * p )
{
   while ( p && (*p!='\0') && ( iscntrl((int) *p) || (*p==' ') || (*p=='\t') ) )
   {
      p++;
   }

   return p;
}


/**
 *****************************************************************************
 * Helper to truncated a string and show the truncation
 * Returns a pointer to the string and sets the truncation buffer pointer if
 *  the string had to be truncated
 *
 * @param   p  String to truncate
 * @param   maxLength   Up to
 * @param   pTruncationMarkBuffer   A string buffer pointer to set to a
 *           truncation string if truncated. Left unchanged if no truncation
 * @return  A pointer to the truncated string, that points to the original
             string
 * @author gpa
 *****************************************************************************
 */
const char *log_get_truncated_string(
   const char *p,
   int maxLength,
   const char * * pTruncationMarkBuffer )
{
   int l = strlen(p) - maxLength - 1 /* \0 */;

   if ( l > 0 )
   {
      *pTruncationMarkBuffer = elipsis;
      return &p[l+sizeof(elipsis)];
   }

   return p;
}


/**
 *****************************************************************************
 * Initialise the debugging library
 * The default settings may be overridden by the environment variable
 *  LOG which contains format identical to the command line options
 * This method is called from the shared library entry function and should
 *  explicitly be called when the static version of the library is used.
 *
 *****************************************************************************
 */
#ifdef LOGGER_SMALL
void _log_init( bool splitPatternOff, const char *log_env_to_copy )
#else
void _log_init( bool splitPatternOff )
#endif
{
   int pslen=0;

   _log_filename = "";
   _log_function = "";
   _log_line     = 0;
   _log_level = LOG_LEVEL_MILE; // Default debugging level
#ifndef LOGGER_HAS_NO_FILE_SUPPORT
   _log_outfile = 0;
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

   // All domain activated
   memset( _log_mask, 0, sizeof(_log_mask) );

   // No domain level filtering
   _log_reset_domain_level_filters();

   // Create the print sync mutex
   _log_mutex_init();

   // Get the name of the process and store */
   log_os_set_process_name( _log_process_name, _LOG_MAX_PROCESS_NAME_LENGTH );

   // Reset the last settings
   memset( (void*)&_log_last, 0, sizeof(_logSetting_t) );
   _log_last.domain = "";

   // Don't modify the LOG env var. in case the application looks at it
   char log_env_copy[_LOG_MAX_COMMAND_LINE_LENGTH];
#  ifndef LOGGER_SMALL
   const char *log_env_to_copy = log_get_config_string();
#  endif

   if ( log_env_to_copy )
   {
      strlcpy( log_env_copy, log_env_to_copy, _LOG_MAX_COMMAND_LINE_LENGTH );
   }
   else
   {
      log_env_copy[0] = '\0';
   }

   // Initialize the debug level from the environment variable
   char *p, *env_log = log_env_copy;

   const char *bad = 0;
   bool spottedEndOfLine;

   // Keep a copy of the duplicated string
   env_log = log_env_copy;

   // Parse the arguments passed on the command line
   for (;;)
   {
      env_log = log_skip_blanks( env_log );
      p = env_log;
      spottedEndOfLine = false;

      if ( !p || !*p )
      {
         break;
      }

      /* Search for the end of a keyword */
      while ( *p && ! strchr(" \t", *p) )
      {
         p++;
      }

      // Terminate
      if ( *p == '\0' )
         spottedEndOfLine = true;
      else
         *p = '\0';

      // Parse
      if ( strcmp( env_log, "error" ) == 0 )
      {
         _log_level = LOG_LEVEL_ERROR;
      }
      else if ( strcmp( env_log, "warn" ) == 0 )
      {
         _log_level = LOG_LEVEL_WARN;
      }
      else if ( strcmp( env_log, "mile" ) == 0 )
      {
         _log_level = LOG_LEVEL_MILE;
      }
      else if ( strcmp( env_log, "info" ) == 0 )
      {
         _log_level = LOG_LEVEL_INFO;
      }
      else if ( strcmp( env_log, "trace" ) == 0 )
      {
         _log_level = LOG_LEVEL_TRACE;
      }
      else if ( strcmp( env_log, "debug" ) == 0 )
      {
         _log_level = LOG_LEVEL_DEBUG;
      }
      else if ( strncmp( env_log, "domain=", 7 ) == 0 )
      {
         _log_set_mask( &(env_log[7]) );
      }
      else if ( strncmp( env_log, "exclude=", 8 ) == 0 )
      {
         _log_set_not_mask( &(env_log[8]) );
      }
      else if ( strncmp( env_log, "level=", 6 ) == 0 )
      {
         short l = '0' - env_log[6];

         if ( (l >= 0) && (l <= LOG_LEVEL_DEBUG) )
         {
            _log_level = (logLevel_t)l;
         }
      }
#ifndef LOGGER_HAS_NO_FILE_SUPPORT
      else if ( strcmp( env_log, "stdout" ) == 0 )
      {
         _log_outfile = stdout;
      }
      else if ( strcmp( env_log, "stderr" ) == 0 )
      {
         _log_outfile = stderr;
      }
      else if ( strncmp( env_log, "file=", 5 ) == 0 )
      {
         char *filename = env_log + 5;

         // Try to open the file
         FILE *pf = fopen( filename, "a" );

         if ( pf != 0 )
         {
             setvbuf( pf, 0, _IONBF, 0 );
            _log_outfile = pf;
         }
      }
      else if ( strncmp( env_log, "file[", 5 ) == 0 )
      {
         // Domain level redirection
         bool badSyntax=true;
         char *pMarker =  env_log + 5;

         // Skip to end of domain list
         while ( *pMarker!=0 && *pMarker!=']' )
         {
            pMarker++;
         }

         if ( *pMarker )
         {
            // Terminate end of domain definition
            *pMarker = '\0';

            // Check next sign is =
            pMarker++;

            if ( *pMarker++ == '=' )
            {
               char *filename = pMarker;

               // Try to open the file
               FILE *pf = fopen( filename, "a" );

               if ( pf != 0 )
               {
                   setvbuf( pf, 0, _IONBF, 0 );
                  _log_set_domain_redirection( env_log + 5, pf);
                  badSyntax = false;
               }
            }
         }

         if ( badSyntax )
         {
            // Bad argument - remember
            bad = "Incorrect file in file[]";
         }
      }
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT
      else if ( strcmp( env_log, "full" ) == 0 )
      {
         _log_full = true;
      }
      else if ( strncmp( env_log, "dom[", 4 ) == 0 )
      {
         // Domain level filtering
         bool badSyntax=true;
         char *pMarker =  env_log + 4;
         logLevel_t domLevel = (logLevel_t)-1;

         // Skip to end of domain list
         while ( *pMarker!=0 && *pMarker!=']' )
         {
            pMarker++;
         }

         if ( *pMarker )
         {
            // Terminate end of domain definition
            *pMarker = '\0';

            // Check next sign is =
            pMarker++;

            if ( *pMarker++ == '=' )
            {
               // Check following is a correct level
               if ( strcmp( pMarker, "error" ) == 0 )
               {
                  domLevel = LOG_LEVEL_ERROR;
               }
               else if ( strcmp( pMarker, "warn" ) == 0 )
               {
                  domLevel = LOG_LEVEL_WARN;
               }
               else if ( strcmp( pMarker, "mile" ) == 0 )
               {
                  domLevel = LOG_LEVEL_MILE;
               }
               else if ( strcmp( pMarker, "info" ) == 0 )
               {
                  domLevel = LOG_LEVEL_INFO;
               }
               else if ( strcmp( pMarker, "trace" ) == 0 )
               {
                  domLevel = LOG_LEVEL_TRACE;
               }
               else if ( strcmp( pMarker, "debug" ) == 0 )
               {
                  domLevel = LOG_LEVEL_DEBUG;
               }
            }

            if ( domLevel > 0 )
            {
               _log_set_domain_level( env_log + 4, domLevel );

               badSyntax = false;
            }
         }

         if ( badSyntax )
         {
            // Bad argument - remember
            bad = "Incorrect level in dom[]";
         }
      }
#ifndef LOGGER_SMALL
      else
      {
         if ( ! log_os_parse_config( env_log ) )
         {
            bad = env_log;
         }
      }
#endif

      if ( spottedEndOfLine )
      {
         break;
      }

      // Skip word just parsed
      env_log = p+1;
   }

   if ( ! splitPatternOff )
   {
      // Display the separation banner with the process name to indicate a new session
      snprintf(
         _log_buffer,
         sizeof(_log_buffer)-1,
         "## %s %s",
         _log_process_name,
         _LOG_SPLIT_PATTERN+pslen );
      _log_print( _log_buffer );
   }

   // If a bad arg was found, add it to the error
   if ( bad )
   {
      LOG_ERROR( "log", "Bad option \"%s\" in LOG environment var", bad );
   }
}


/**
 *****************************************************************************
 * Return any current limits in use
 *
 * @param   ID of the limit to retreive
 * @return  The limit, or -1 if the limit is not valid or out of context
 * @see logLimit_t
 *****************************************************************************
 */
long _log_get_limit( logLimit_t type )
{
   switch( type )
   {
      case LOG_LIMIT_TRACE:                return _LOG_MAX_TRACE;
      case LOG_LIMIT_DOMAIN_REPR:          return _LOG_MAX_DOMAIN_REPR_LENGTH;
      case LOG_LIMIT_FUNCTION_REPR:        return _LOG_MAX_FUNCTION_REPR_LENGTH;
      case LOG_LIMIT_FILE_REPR:            return _LOG_MAX_FILE_REPR_LENGTH;
      case LOG_LIMIT_PROCESS_REPR:         return _LOG_MAX_PROCESS_NAME_LENGTH;
      case LOG_LIMIT_THREAD_ID_REPR:       return _LOG_MAX_THREAD_ID_REPR_LENGTH;
      case LOG_LIMIT_LINE_REPR:            return _LOG_MAX_LINE_NUMBER_REPR_LENGTH;
      case LOG_LIMIT_DOMAIN_LEVEL_FILTERS: return _LOG_MAX_DOMAIN_LEVEL_FILTERS;
      case LOG_LIMIT_DOMAINS:              return _LOG_MAX_DOMAINS;
      case LOG_LIMIT_MASKS:                return _LOG_MAX_DOMAINS*(_LOG_MAX_DOMAIN_REPR_LENGTH+1);
      default:
         return -1;
   }
}


/**
 *****************************************************************************
 * Check if the domain is within the current mask and not in the not mask
 *
 * @param   mask  Single domain name to check
 * @return  true if it is, and the domain should be displayed
 *****************************************************************************
 */
bool _log_is_set( const char *mask )
{
   _logDomainIdentifier_t *pIncluded = _log_mask;

   // First element is empty - all domains are shown beside hidden ones
   if ( pIncluded->name[0] == '\0' )
   {
      // There are no domain set. We must check if there are any bared domains
      _logDomainIdentifier_t *pExcluded = _log_not_mask;
      int score = 0;

      while ( pExcluded->name[0] != '\0' )
      {
         if ( wildcmp(pExcluded->name, mask, &score) )
         {
            // Found in the hide - do not show
            return false;
         }

         ++pExcluded;
      }

      // Empty mask and not in the hide section = show
      return true;
   }
   else // A list of active domains exists. Only show domains in that list
   {
      int score = 0;

      do
      {
         if ( wildcmp(pIncluded->name, mask, &score) )
         {
            // A domain list exists and the domain is in - show
            return true;
         }

         ++pIncluded;
      } while ( pIncluded->name[0] != '\0' );
   }

   // A domain list exists but the domain is not listed - do not show
   return false;
}


/**
 * Quick check to see if domain filtering is activated.
 * Assumes the lock is taken.
 * The lock is left untouched.
 *
 * @param domain The domain to filter
 * @return true to indicate that the domain is to proceed.
 *         false means that domain should not be displayed
 */
bool _log_filter_trace( const char *domain, logLevel_t logLevel )
{
   // Empty domain means display in all cases.
   if ( ! domain )
   {
      return _log_level >= logLevel;
   }

   // Short cut for 99% of cases where there are no domain level filtering
   if ( _LOG_DOMAIN_LEVEL_LOOKUP_IS_EMPTY )
   {
      // If the level is OK
      if ( _log_level >= logLevel )
      {
         // Check domain in show list or not in hide list
         return _log_is_set(domain);
      }

      return false;
   }

   // Check to see if the domain has a level associated
   // Look for the domain in the list
   // Since the wildcard can yield many match at different level, we
   //  go through all filters and keep the best match
   int matches = 0;
   logLevel_t level = _log_level;
   int bestScore = 0;
   size_t i;

   for (
      i=0;
      i<_LOG_MAX_DOMAIN_LEVEL_FILTERS && _log_domain_level_lookup[i].domain.name[0];
      ++i )
   {
      if ( wildcmp( _log_domain_level_lookup[i].domain.name, domain, &bestScore ) )
      {
         ++matches;
         level=_log_domain_level_lookup[i].level;
      }
   }

   if (matches)
   {
      // The domain is in the list - Check the level and return now !
      return ( level >= logLevel );
   }

   // We're back where we started. The domain list is not empty, but this domain
   //  is not affected
   // If the level is OK
   if ( _log_level >= logLevel )
   {
      // Check domain in show list or not in hide list
      return _log_is_set(domain);
   }

   return false;
}

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
/**
 * Update #_log_outfile if domain is in #_log_domain_file_lookup
 *
 * @param domain The domain to lookup
 */
void _log_redirect_outfile( const char *domain )
{
   FILE *f = 0;

   // Check to see if the domain has a file associated
   // Look for the domain in the list
   // Since the wildcard can yield many match at different level, we
   //  go through all filters and keep the best match
   int matches = 0;
   int bestScore = 0;
   size_t i;

   for (
      i=0;
      i<_LOG_MAX_DOMAIN_FILE_REDIRECTION && _log_domain_file_lookup[i].domain.name[0];
      ++i )
   {
      if ( wildcmp( _log_domain_file_lookup[i].domain.name, domain, &bestScore ) )
      {
         ++matches;
         f = _log_domain_file_lookup[i].pf;
      }
   }

   if (matches)
   {
      // The domain is in the list, update _log_outfile
      _log_outfile = f;
   }

}
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

/**
 *****************************************************************************
 * Actually generates the output
 * <b>Warning</b> This API should only be called from one of the macros as it
 *  will unlock the global mutex. Also, if you modify the code and add an exit
 *  point, remember to unlock the mutex.
 *
 * @param domain   Text name of concerned domains or NULL for
 *                 reserved domains used by LOG_ASSERT
 *                 like messages and are always displayed
 * @param format   Printf format string
 * @param ...      Format arguments
 *****************************************************************************
 */
void _log_trace(const char* domain, const char *format, ...)
{
#ifndef LOGGER_HAS_NO_FILE_SUPPORT
   FILE *bkupOutfile;
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

   enum {
      eCATEGORY_MARKER = 0,
      eTHREAD_ID,
      eDOMAIN_SPACER,
      eFILENAME_SPACER,
      eFUNCTION_SPACER,
      eSTART_OF_STRING,
      eDOMAIN_IN,
      eDOMAIN_TRUNCATION,
      eDOMAIN,
      eDOMAIN_OUT,
      eFILENAME_TRUNCATION,
      eFILENAME,
      ePOST_FILENAME,
      eFUNCTION_NAME_TRUNCATION,
      eFUNCTION_NAME,
      ePOST_FUNCTION_NAME,
      eEND
   };

   if ( ! _log_filter_trace(domain, _log_type) )
   {
      _log_unlock();
      return;
   }

   const char * const oneBlankSpace=" ";

   _LOG_THREAD_ID current_thread;

   static const char* p[eEND+1];
   static char threadIdBuffer[_LOG_SIZED_FOR(_LOG_MAX_THREAD_ID_REPR_LENGTH)];
   static char lineNumberBuffer[_LOG_SIZED_FOR(_LOG_MAX_LINE_NUMBER_REPR_LENGTH)];

   // The final debug is concatenation of segmented strings
   // Reset pointer
   memset( (char *)p, 0,  sizeof(p) );

   // Obtain current thread id
   current_thread = _log_get_current_thread_id();

   if ( ! domain )
   {
      domain = "ALERT";
   }

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
   // Before printing, change _log_outfile if required
   bkupOutfile = _log_outfile;
   _log_redirect_outfile(domain);

#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

   /*
    *   Add timestamp
    */

   // Is there a callback for getting a timestamp?
   unsigned long epoch = 0;
   if ( _log_datetime_callback )
   {
      epoch = _log_datetime_callback();
   }

   const char *timeString = log_os_timestamp(epoch);

   if ( timeString )
   {
      _log_print( timeString );
   }

   switch ( _log_type )
   {
      case LOG_LEVEL_ERROR : p[eCATEGORY_MARKER] = "#"; break;
      case LOG_LEVEL_WARN  : p[eCATEGORY_MARKER] = "!"; break;
      case LOG_LEVEL_MILE  : p[eCATEGORY_MARKER] = "+"; break;
      case LOG_LEVEL_INFO  : p[eCATEGORY_MARKER] = ">"; break;
      case LOG_LEVEL_TRACE : p[eCATEGORY_MARKER] = "-"; break;
      case LOG_LEVEL_DEBUG : p[eCATEGORY_MARKER] = "."; break;
      default:               p[eCATEGORY_MARKER] = " "; break;
   }

   // Push the thread id to clearly mark context switch
   if ( _log_full || _log_last.thread_id != current_thread )
   {
      _log_last.thread_id = current_thread;

      _log_copy_thread_id(threadIdBuffer, sizeof(threadIdBuffer), current_thread );

      p[eTHREAD_ID] = threadIdBuffer;
   }
   else
   {
      // If no context switch, adjust indentation
      if ( strncmp(_log_last.domain, domain, _LOG_MAX_DOMAIN_REPR_LENGTH) == 0 )
         p[eDOMAIN_SPACER]=oneBlankSpace;

      if ( _log_last.filename == _log_filename )
         p[eFILENAME_SPACER]=oneBlankSpace;

      if (_log_last.function == _log_function)
         p[eFUNCTION_SPACER]=oneBlankSpace;
   }

   // Start the string
   p[eSTART_OF_STRING]="{";

   // Display domain
   if ( _log_full || ! p[eDOMAIN_SPACER] )
   {
      _log_last.domain = domain;
      p[eDOMAIN_IN]="<";
      p[eDOMAIN] = log_get_truncated_string(
         domain,
         _LOG_MAX_DOMAIN_REPR_LENGTH,
         &p[eDOMAIN_TRUNCATION] );
      p[eDOMAIN_OUT]=">";
   }

   // Adjust filename
   if ( _log_full || _log_last.filename != _log_filename )
   {
      _log_last.filename = _log_filename;

      p[eFILENAME] = log_get_truncated_string(
         _log_filename,
         _LOG_MAX_FILE_REPR_LENGTH,
         &p[eFILENAME_TRUNCATION] );

      p[ePOST_FILENAME] = oneBlankSpace;
   }

   if ( _log_full || _log_last.function != _log_function )
   {
      _log_last.function = _log_function;

      p[eFUNCTION_NAME] = log_get_truncated_string(
         _log_function,
         _LOG_MAX_FUNCTION_REPR_LENGTH,
         &p[eFUNCTION_NAME_TRUNCATION] );

      p[ePOST_FUNCTION_NAME] = oneBlankSpace;
   }

   // Terminate with the line number and a quote
   snprintf( lineNumberBuffer, sizeof(lineNumberBuffer), "%lu} ", _log_line );
   p[eEND] = lineNumberBuffer;

   // Add arguments
   va_list pList;
   int count=0;

   // Prepend with the header
   // Yes, I know - register is useless - but it makes me feel good
   char *pTo = _log_buffer;
   register size_t i;

   for ( i=0; i<=eEND; ++i )
   {
      register const char *pFrom = p[i];

      while ( pFrom && *pFrom )
      {
         *pTo++ = *pFrom++;
         ++count;
      }
   }

   // Take the va args and write formatted to our buffer, avoiding going over
   va_start( pList, format );
   vsnprintf( pTo, sizeof(_log_buffer)-count, format, pList );
   va_end( pList );

   // Terminate the buffer in case the string got truncated
   _log_buffer[ sizeof(_log_buffer) - 1 ] = '\0';
   _log_print( _log_buffer );

   // Inject into external logger too
   if ( _log_logger_callback )
   {
      _log_logger_callback(
         _log_line,
         _log_filename,
         _log_function,
         current_thread,
         _log_type,
         domain,
         _log_buffer
      );
   }

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
   if ( _log_outfile != 0 )
   {
      fflush( _log_outfile );
   }

   // Restore _log_outfile
   _log_outfile = bkupOutfile;
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

   // The actual trace was locked in TRACE/LOG/WARN macros.
   _log_unlock();
}

/**
 * Simple printf for raw print. Use like a printf function.
 * Trim any \r\n at the very end of the string since it gets added
 */
void _log_printf(const char *format, ...)
{
   va_list pList;

   _log_lock();

   // Write the lot
   va_start( pList, format );

   // For info, vsnprintf returns the number of characters that would have been written
   //  if n had been sufficiently large, not counting the terminating null character
   int sizeRequired = vsnprintf( _log_buffer, sizeof(_log_buffer), format, pList );
   va_end( pList );

   if ( sizeRequired > 0 )
   {
      // Trim the \n and \r at the end of the buffer. Only the last one
      char *pLastChar = _log_buffer + sizeRequired - 1;
      char lastChar = *pLastChar; // Grab the last char

      if (lastChar == '\r' || lastChar == '\n')
      {
         // Trim the line
         --sizeRequired;
         *pLastChar = '\0';

         // Look for a '\n' after a '\r' or a '\r' after a '\n' if we have more chars
         if ( sizeRequired > 0 )
         {
            // Move onto the next char
            --pLastChar;

            // Look for the preceeding char
            if ( (*pLastChar == '\r' || *pLastChar == '\n') && *pLastChar != lastChar )
            {
               // Trim again
               --sizeRequired;
               *pLastChar = '\0';
            }
         }
      }
   }

   // Failed to copy all - or the terminating character
   if ( sizeof(_log_buffer) < (unsigned int)sizeRequired )
   {
      // Add an elipsis to mark the truncation
      strcpy( _log_buffer + sizeof(_log_buffer) - sizeof(elipsis), elipsis );
   }

   // Indicate truncation Terminate the buffer in case the string got truncated
   _log_buffer[ sizeof(_log_buffer) - 1 ] = '\0';
   _log_print( _log_buffer );

   _log_unlock();

   // Now we're unlock, warn if vsnprintf failed
   if ( sizeRequired < 0 )
   {
      LOG_WARN(0, "String format is invalid");
   }
}

/**
 *****************************************************************************
 * Actually generates the output at error level
 * <b>Warning</b> This API should only be called from one of the macros as it
 *  will unlock the global mutex. Also, if you modify the code and add an exit
 *  point, remember to unlock the mutex.
 *
 * @param domain   Text name of concerned domains or NULL for
 *                 reserved domains used by LOG_ASSERT
 *                 like messages and are always displayed
 * @param str      String to be logged
 *****************************************************************************
 */
void _log_trace_error(const char* domain, const char *str )
{
   // Force error level
   _log_type = LOG_LEVEL_ERROR;
   _log_trace(domain, "%s", str);
}

/**
 *****************************************************************************
 * Helper which creates a list of domain identifier from a string
 *
 * @param mask   Text masks separated by an obvious separator char or null
 *                to reset the list. The list is always fully set from the
 *                given mask.
 * @param list   Receiving list with the identifiers
 * @param max    Maximun number of domains to consider
 *****************************************************************************
 */
static void _log_apply_mask_to_list(
   const char *mask, _logDomainIdentifier_t *list, size_t max )
{
   char buf[_LOG_MAX_DOMAIN_LIST_LENGTH];
   char *token, *p = NULL;

   // We do not want to change the mask half way through a trace !
   _log_lock();

   // Reset mask list
   memset( list, 0, sizeof(_logDomainIdentifier_t) * max );

   if ( mask != 0 )
   {
      strlcpy( buf, mask, _LOG_MAX_DOMAIN_LIST_LENGTH );
      token = log_strtok_r( buf, seps, &p );

      while ( token != NULL )
      {
         size_t i;

         for ( i=0; i!=max; i++)
         {
            if ( list[i].name[0] == '\0' )
            {
               strlcpy( list[i].name, token, _LOG_MAX_DOMAIN_REPR_LENGTH );

               // Now remove from the list of domains level.
               // A domain is either shown, hidden, or leveled but not a combination
               _log_clear_domain_level(token);

               break;
            }
         }

         token = log_strtok_r( NULL, seps, &p );
      }
   }

   // Unlock operation
   _log_unlock();
}


/**
 *****************************************************************************
 * Helper which creates a colon separated string from a list of domain
 *
 * @param list   Receiving list with the identifiers
 * @param dest   String the receive the mask description
 * @param max    Max length of string buffer
 *****************************************************************************
 */
static void _log_copy_mask_to_string(
   _logDomainIdentifier_t *list, char *dest, size_t max )
{
   bool again = false;

   if ( max )
   {
      // Recurse through the domain list for as long as there is room in the buffer
      while ( list->name[0] != '\0' && max )
      {
         const char *dom = list->name;

         // Split using a ':'
         if ( again )
         {
            *dest++ = ':';
            --max;
         }

         // Copy domain name
         while ( max && (*dom) )
         {
            *dest++ = *dom++;
            --max;
         }

         // Next domain in the list
         ++list;

         // Remember to add a ':' if there is another domain
         again = true;
      }

      // Terminate buffer string as a courtesy
      if ( max )
      {
         *dest = '\0';
      }
   }
}


/**
 *****************************************************************************
 * Helper which removes the given mask from a list of masks
 *
 * @param list   List with the identifiers to modify
 * @param dom    String containing the domain to remove
 *****************************************************************************
 */
static void _log_remove_mask_from_list( _logDomainIdentifier_t *list, char *dom )
{
   size_t i = 0;

   // Recurse through the domain list or until we reach bottom
   while ( list->name[i] != '\0' )
   {
      size_t j = i;

      if ( strcmp( list->name, dom ) == 0 )
      {
         // Found it. Now get the last domain in the list
         do
         {
            ++j;
         } while ( list->name[j] != '\0' );

         if ( --j != i )
         {
            // Swap the last and the current
            memcpy( &list[i], &list[j], sizeof(_logDomainIdentifier_t) );
         }

         // Reset the j slot
         list->name[j] = '\0';

         break;
      }

      ++i;
   }
}

/**
 *****************************************************************************
 * Set the domain mask, i.e. which domains traces will be printed.
 *
 * @param mask   Text masks separated by an obvious separator character
 *****************************************************************************
 */
void _log_set_mask( const char *mask )
{
   // Create a list of show
   _log_apply_mask_to_list( mask, _log_mask, _LOG_MAX_DOMAINS );

   // Reset all hide domains
   _log_apply_mask_to_list( NULL, _log_not_mask, _LOG_MAX_DOMAINS );
}


/**
 *****************************************************************************
 * Set the excluded domain mask, i.e. which domains traces will
 *  NOT be printed.
 *
 * @param mask   Text masks separated by an obvious separator character
 *****************************************************************************
 */
void _log_set_not_mask( const char *mask )
{
   // Create a list of hide
   _log_apply_mask_to_list( mask, _log_not_mask, _LOG_MAX_DOMAINS );

   // Reset all show domains
   _log_apply_mask_to_list( NULL, _log_mask, _LOG_MAX_DOMAINS );
}


/**
 *****************************************************************************
 * Return a copy of the masking status.
 * The caller is encouraged to use the method LOG_GETLIMIT to get a safe size
 *  for either string. (use LOG_LIMIT_MASKS). If either l1 or l2 or set to 0,
 *  the function will not touch the mask buffer. This can be used to only
 *  retreive 1 mask at a time.
 * <pre>Example:
 * size_t l = std::min( LOG_GETLIMIT( LOG_LIMIT_MASKS ), 512 );
 * char mask[l];
 * LOG_GETMASKS( mask, l, 0, 0 );  // only get the mask
 * ...
 * LOG_GETMASKS( 0, 0, mask, l );  // and now the not mask
 * </pre>
 *
 * @param masks   Pointer the string buffer where a colon separated string
 *                 containing the mask gets copied for up to l1 char. A \0 is
 *                 inserted if the string length permits.
 * @param lMasks Maximum length of the buffer receiving the masks
 * @param notMask As per masks for the domains to hide and using l2 to limit.
 * @param lNotMasks Maximum length of the buffer receiving the masks
 *****************************************************************************
 */
void _log_get_masks(
   char *masks, size_t lMasks, char *notMasks, size_t lNotMasks )
{
   _log_lock();

   _log_copy_mask_to_string( _log_mask, masks, lMasks );
   _log_copy_mask_to_string( _log_not_mask, notMasks, lNotMasks );

   _log_unlock();
}


/**
 *****************************************************************************
 * Set the overall debugging level
 * A level set to LOG_TRACE will block LOG_DEBUG, but let LOG_WARN and
 *  LOG_TRACE through etc...
 *
 * @param Level   New debugging level
 * @see   log.h for logLevel_t definition.
 *****************************************************************************
 */
void _log_set_level( logLevel_t level )
{
   _log_lock();

   if ( level <= LOG_LEVEL_DEBUG )
   {
      _log_level = level;
   }

   _log_unlock();
}


/**
 *****************************************************************************
 * Return the current debugging level.
 *
 * @return  Current debugging level
 *****************************************************************************
 */
logLevel_t _log_get_level()
{
   // No lock, the type is atomic
   return _log_level;
}


/**
 *****************************************************************************
 * Given a list of domains, set the level of those domains to the given level
 * There only up to #_LOG_MAX_DOMAIN_LEVEL_FILTERS domain which can be setup
 *  this way. If the list is full, the function returns 0.
 * To reset the list, simply set a null domain to any level.
 *
 * @param domains   A string with domains as strings, separated by an
 *                   appropriate separators. A null pointer or empty string
 *                   will have no effect.
 * @param level     Appropriate debug level to use from now on on the domains
 *
 * @return  The number of domains actually set. If 0, no domains were set as
 *           the list is full
 *****************************************************************************
 */
int _log_set_domain_level( const char *domains, logLevel_t level )
{
   char *token, *p = NULL;
   int actuallySet = 0;

   // We do not want to change the mask half way through a trace !
   _log_lock();

   if ( domains && domains[0] != '\0' )
   {
      char buf[_LOG_MAX_DOMAIN_LIST_LENGTH];

      strlcpy( buf, domains, _LOG_MAX_DOMAIN_LIST_LENGTH );
      token = log_strtok_r( buf, seps, &p );

      while ( token != 0 )
      {
         bool bAlreadySet = false;
         size_t i;

         // Check to see if the specified domain is in the list
         for (
            i=0;
            i<_LOG_MAX_DOMAIN_LEVEL_FILTERS && _log_domain_level_lookup[i].domain.name[0];
            ++i )
         {
            if ( strncmp(
               _log_domain_level_lookup[i].domain.name,
               token,
               _LOG_MAX_DOMAIN_REPR_LENGTH ) == 0 )
            {
               // Set new level
               _log_domain_level_lookup[i].level = level;
               actuallySet++;
               bAlreadySet = true;
            }
         }

         if ( ! bAlreadySet )
         {
            size_t i;

            // Add to list - if possible
            for ( i=0; i<_LOG_MAX_DOMAIN_LEVEL_FILTERS; i++)
            {
               if ( ! _log_domain_level_lookup[i].domain.name[0] )
               {
                  strlcpy(
                     _log_domain_level_lookup[i].domain.name,
                     token,
                     _LOG_MAX_DOMAIN_REPR_LENGTH );

                  _log_domain_level_lookup[i].level = level;
                  actuallySet++;

                  break;
               }
            }
         }

         // Now clear from the domains
         _log_remove_mask_from_list( _log_mask, token );
         _log_remove_mask_from_list( _log_not_mask, token );

         token = log_strtok_r( NULL, seps, &p );
      }
   }

   _log_unlock();

   return actuallySet;
}

/**
 *****************************************************************************
 * Given a list of domains, clear the level logging for those domains.
 *
 * @param domains   A string with domains as strings, separated by an
 *                   appropriate separators. A null pointer or empty string
 *                   will reset all current settings.
 *
 * @return  The number of domains actually set
 *****************************************************************************
 */
int _log_clear_domain_level( const char *domains )
{
   char *token, *p = NULL;
   int actuallyCleared = 0;

   // We do not want to change the mask half way through a trace !
   _log_lock();

   // If the requested clear is empty, clear all
   if ( domains == 0 || domains[0] == '\0' )
   {
      _log_reset_domain_level_filters();
   }
   else
   {
      char buf[_LOG_MAX_DOMAIN_LIST_LENGTH];

      strlcpy( buf, domains, _LOG_MAX_DOMAIN_LIST_LENGTH );
      token = log_strtok_r( buf, seps, &p );

      while ( token != 0 )
      {
         size_t i;

         // Check to see if the specified domain is in the list
         for (
            i=0;
            i<_LOG_MAX_DOMAIN_LEVEL_FILTERS && _log_domain_level_lookup[i].domain.name[0];
            ++i )
         {
            if ( strncmp(
               _log_domain_level_lookup[i].domain.name,
               token,
               _LOG_MAX_DOMAIN_REPR_LENGTH ) == 0 )
            {
               // Free the slot
               // Since we don't want to leave holes (that would mean no domains)
               //  we shift the last active slot into the deleted one

               // Find (if any) the last slot
               size_t j=_LOG_MAX_DOMAIN_LEVEL_FILTERS;

               while ( --j != i )
               {
                  if (  _log_domain_level_lookup[j].domain.name[0] != '\0' )
                  {
                     // This slot (from the bottom is used
                     memcpy(
                        &_log_domain_level_lookup[i], /* dest */
                        &_log_domain_level_lookup[j], /* src */
                        sizeof(_logDomainLevelPair_t) );

                     break;
                  }
               }

               // Free the slot (moved or not)
               _log_domain_level_lookup[j].domain.name[0] = '\0';

               ++actuallyCleared;
               break;
            }
         }

         token = log_strtok_r( NULL, seps, &p );
      }
   }

   _log_unlock();

   return actuallyCleared;
}

/**
 *****************************************************************************
 * Return the domain filtering list. Useful for a trace monitor client.
 * The supplied list is first cleared. So old list can be reused.
 * The domain name is copied inside the structure, so make sure to allocate
 *  enough characters.
 * Although a limit can be specified for both the domain string length and/or
 *  the max number of domains, the caller is encouraged to use the method
 *  LOG_GETLIMIT to extract the system limits.
 * The list is filled with valid domain names. A empty string name indicate
 *  the end of the list.
 *
 * @param map  A list of domain level pairs to be filled
 * @param maxNameLength  Max number of character per domain name (including null terminator)
 * @param maxDomains     Max number of element to copy in list
 *
 * @return  The number of set domains copied in list
 * @see LOG_GETLIMIT
 *****************************************************************************
 */
int _log_get_domain_levels(
   logDomainLevelPair_t* map, size_t maxNameLength, size_t maxDomains )
{
   int found = 0;
   size_t i, j;


   // Copy from reference list
   _log_lock();

   for ( i=0, j=0; (i < _LOG_MAX_DOMAIN_LEVEL_FILTERS) && (i < maxDomains); ++i)
   {
      if ( _log_domain_level_lookup[i].domain.name[0] )
      {
         strlcpy( map[j].domain, _log_domain_level_lookup[i].domain.name, maxNameLength -1 );
         map[j].domain[maxNameLength - 1] = '\0';
         map[j].level = _log_domain_level_lookup[i].level;
         ++j, ++found;
      }
   }

   _log_unlock();

   return found;
}

#ifndef LOGGER_HAS_NO_FILE_SUPPORT
/**
 *****************************************************************************
 * Given a list of domains, set the redirection file of those domains to the given file
 * There only up to #_LOG_MAX_DOMAIN_FILE_REDIRECTION domain which can be setup
 *  this way. If the list is full, the function returns 0.
 * To reset the list, simply set a null domain to any level.
 *
 * @param domains   A string with domains as strings, separated by an
 *                   appropriate separators. A null pointer or empty string
 *                   will have no effect.
 * @param pf        Appropriate pointer on file descriptor to use from now on on the domains
 *
 * @return  The number of domains actually set. If 0, no domains were set as
 *           the list is full
 *****************************************************************************
 */
int _log_set_domain_redirection( const char *domains, FILE *pf )
{
   char *token, *p = NULL;
   int actuallySet = 0;

   // We do not want to change the mask half way through a trace !
   _log_lock();

   if ( domains && domains[0] != '\0' )
   {
      char buf[_LOG_MAX_DOMAIN_LIST_LENGTH];

      strlcpy( buf, domains, _LOG_MAX_DOMAIN_LIST_LENGTH );
      token = log_strtok_r( buf, seps, &p );

      while ( token != 0 )
      {
         bool bAlreadySet = false;
         size_t i;

         // Check to see if the specified domain is in the list
         for (
            i=0;
            i<_LOG_MAX_DOMAIN_FILE_REDIRECTION && _log_domain_file_lookup[i].domain.name[0];
            ++i )
         {
            if ( strncmp(
               _log_domain_file_lookup[i].domain.name,
               token,
               _LOG_MAX_DOMAIN_REPR_LENGTH ) == 0 )
            {
               // Set new level
               _log_domain_file_lookup[i].pf = pf;
               actuallySet++;
               bAlreadySet = true;
            }
         }

         if ( ! bAlreadySet )
         {
            // Add to list - if possible
            for ( i=0; i<_LOG_MAX_DOMAIN_FILE_REDIRECTION; i++)
            {
               if ( ! _log_domain_file_lookup[i].domain.name[0] )
               {
                  strlcpy(
                     _log_domain_file_lookup[i].domain.name,
                     token,
                     _LOG_MAX_DOMAIN_REPR_LENGTH );

                  _log_domain_file_lookup[i].pf = pf;
                  actuallySet++;

                  break;
               }
            }
         }

         token = log_strtok_r( NULL, seps, &p );
      }
   }

   _log_unlock();

   return actuallySet;
}
#endif // ndef LOGGER_HAS_NO_FILE_SUPPORT

/**
 * Allow an external application to intercept all statements being logged.
 * This is on top of the existing logging which cannot be stopped.
 * This call is made inside the lock of the logger, so the callback better
 *  exit quickly.
 *
 * @param cb A callback function which is called for every filtered log statements.
 */
void _log_set_external_logger( LogCallback_t cb )
{
   _log_lock();

   _log_logger_callback = cb;

   _log_unlock();
}

/**
 * Allow an external application to force the timestamping to the logger
 * This can be usefull on system where the application manages the date
 *  and the time independently of the OS time.
 *
 * @param cb A callback function which returns an epoch time.
 */
void _log_set_datetime_callback( LogDatetime_t cb )
{
   _log_lock();

   _log_datetime_callback = cb;

   _log_unlock();
}

/**
 * Default function to copy the thread id
 * As the function is weak - override to set a name using the OS functions
 */
__attribute__((__weak__)) void _log_copy_thread_id( char *buffer, size_t count, _LOG_THREAD_ID id)
{
   snprintf( buffer, count, " 0x%lx ", id );
}


/* ---------------------------- End of file ------------------------------- */
