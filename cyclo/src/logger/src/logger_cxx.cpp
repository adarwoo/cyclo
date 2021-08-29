/*-
 *****************************************************************************
 * Implementation of some of the program logging/tracing functionality to help
 * debugging.
 * This is the common base for all OSes
 *
 * @author gpa
 *****************************************************************************
 */


//----------------------------------------------------------------------------
//  Local include
//----------------------------------------------------------------------------
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <cassert>

#include "logger.h"
#include "logger/logger_os.h"
#include "logger/logger_limits.h"

#ifdef __GNUC__
#  include <cxxabi.h>
#endif

//----------------------------------------------------------------------------
//  Local variables
//----------------------------------------------------------------------------

// Defined in logger_common.c
extern "C"
{
   /** Replacement function for strtok_r which is non ANSI */
   char *log_strtok_r(char *s, const char *delim, char **last);

   /** Replacement for strncpy which accepts array and always terminate the string */
   size_t strlcpy(char *dst, const char *src, size_t size);

   /** Helper to truncated a string and show the truncation */
   const char *log_get_truncated_string( const char *, int, const char ** );

   bool _log_filter_trace( const char *domain, logLevel_t logLevel );
   void _log_reset_last_function();

}  //  End of extern "C"

namespace
{
   /**
    * Local helper to manage demangling a gcc names
    * To avoid malloc and free for every call, we keep a single
    *  buffer which may be reallocated by the abi API.
    * This instance must be static, and the dtor will take care of
    *  the memory cleanup at the end.
    * Note: This class is not reentrant. The global mutex must be locked.
    */
   class Abi_demangler
   {
      /** We need to keep the size of the buffer since the abi API needs it */
      size_t bufferLength;
      /** Actual buffer. Could be realloced */
      char *buffer;

      static const int INVALID_MANGLED_NAME = -2;
      static const int MEM_ALLOC_FAILURE = -1;

   public:
      Abi_demangler() : bufferLength(0), buffer(0) {}

      ~Abi_demangler()
         { free(buffer); }

      char *demangle( const char *className )
      {
         char *result(0);
#ifdef __GNUC__
         int status(0);

         // Demangle the name with the GNU C
         result = abi::__cxa_demangle(className, buffer, &bufferLength, &status );

         switch ( status )
         {
         case 0: // All ok. Memory has been allocated
            // Bug http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42230
            // When the buffer is realloced, the length is set to 0
            // Fix by getting the string length if 0. So should work after bug fixed.
            if ( bufferLength == 0 )
            {
               // Far from ideal as could be smaller but won't leak
               bufferLength = strlen(result) + 1;
            }

            buffer = result;
            break;
         case MEM_ALLOC_FAILURE:
            _log_print("Out of memory!!");
            abort();
         case INVALID_MANGLED_NAME:
            result=0;
            break;
         // The only remaining case is when the parameters are invalid!
         default:
            abort();
         }
#endif

         // Allocate some memory the very first time
         if ( bufferLength == 0 )
         {
            bufferLength = _LOG_SIZED_FOR(_LOG_MAX_FUNCTION_REPR_LENGTH);
            buffer = (char *)malloc(bufferLength);

            // Out of memory?
            if ( buffer == 0 )
            {
               abort();
            }
         }

         if ( result == 0 )
         {
            // If we cannot demangle, print the ugly name. Truncate to correct size.
            strlcpy( buffer, className, _LOG_SIZED_FOR(_LOG_MAX_FUNCTION_REPR_LENGTH) );
         }

         return buffer;
      }
   };
}


/**
 *****************************************************************************
 * Implementation of the logScopeObj constructor.
 * The constructor stores the mask in m_mask and calls the out method with
 *  IN as the string
 *
 * @param line       Line number, filled in by the macro
 * @param file       File name, filled in by the macro
 * @param function   Function name, filled in by the macro
 * @param pThis      this pointer
 * @param classname  Name of the class as reported through the RTTI
 * @param mask       The debugging mask
 *****************************************************************************
 */
_logScopeObj_c::_logScopeObj_c(
   unsigned long line,
   const char   *file,
   const char   *function,
   const void   *pThis,
   const char   *classname,
   const char   *mask )
{
   _log_lock();

   _log_type = LOG_LEVEL_TRACE;

   if ( _log_filter_trace(mask, _log_type) )
   {
       m_line=line;
       m_file=file;
       m_function=function;
       m_this=pThis;
       m_classname=classname;
       m_mask=mask;

       this->out( "--> IN" );
   }
   else
   {
      m_line = 0;
   }

   _log_unlock();
}


/**
 *****************************************************************************
 * Implementation of the logScopeObj destructor.
 * It calls the out method with OUT as the string
 *
 * @note   Since the destructor is called internally be the compiler, no line
 *         number will be available. The filename is rembered from the
 *         constructor.
 *****************************************************************************
 */
_logScopeObj_c::~_logScopeObj_c()
{
   if ( m_line > 0 )
   {
      // Mark the line number as unknown (nobody's perfect)
      m_line = 0;

      this->out( "<-- OUT" );
   }
}

/**
 * Create a single instance of the demangler
 */
static Abi_demangler _log_demangler;


/**
 *****************************************************************************
 * Dumps the header/footer statement on the string taking a string which
 *  specifies whether it is a IN or OUT operation.
 * On gcc system, the function should be complete.
 *
 * @notes   Rtti must be active
 * @notes   The function reads m_this and if not zero, assumes it is an
 *           object pointer. It then uses the RTTI to compute the function
 *           name.
 *
 * @param   strInOrOut   A string which gets appended to the trace.
 *****************************************************************************
 */
void _logScopeObj_c::out( const char *strInOrOut )
{
   enum _item_
   {
      eCLASSNAME_TRUNCATION=0,
      eCLASSNAME,
      eDOUBLE_COLON_SEPARATOR,
      eDESTRUCTOR_TILDE,
      eMETHODNAME_TRUNCATION,
      eMETHODNAME,
      eEND
   };

   // Split the repr into 2 pieces
   // The function name is made of the class name and the method sep'd by a ::
   //                           123456
   static char THIS_MARKER[] = " this=%.8p";
   //                                 0x0123456789ABCDEF - Up to 64 bits
   //                          -----------------
   //                           123456789012345678901234 = 24
   static const size_t THIS_MARKER_REPR_LENGTH = 24;

   // The double colon separator
   static char DOUBLE_COLON[] = "::";

   // It takes 2 chars to display '::'
   static const size_t DOUBLE_COLON_REPR_LENGTH = 2;

   static const size_t MAX_CLASS_METHOD_UNDECORATED_REPR =
      _LOG_MAX_FUNCTION_REPR_LENGTH - THIS_MARKER_REPR_LENGTH - DOUBLE_COLON_REPR_LENGTH;

   // Give even space to the class name and the method name
   static const size_t MAX_CLASSNAME_REPR(
      MAX_CLASS_METHOD_UNDECORATED_REPR / 2 );

   // Cope for odd length such that the method name could be 1 more than the class name
   static const size_t MAX_METHODNAME_REPR(
      MAX_CLASS_METHOD_UNDECORATED_REPR - MAX_CLASSNAME_REPR );

   // Store the address of the object
   static char thisBuffer[THIS_MARKER_REPR_LENGTH+1];

   // Keep the name in chunks
   static const char* p[eEND+1];

   // The demangler is not reentrant !!
   _log_lock();   // Trace will unlock

   // The final debug is concatenation of segmented strings
   // Reset pointer
   memset( p, 0, sizeof(p));

   // Are we inside an object?
   if ( m_classname != 0 )
   {
      char *className = _log_demangler.demangle(m_classname);

      // Skip front numbers which GNU inserts
      while ( *className >= '0' && *className <= '9' )
      {
         className++;
      }

      // Work out if one of the segments is shorter than the allocated max length
      //  and redistribute the space
      size_t sparesForClassName(0);
      // Extra chars to allocate to the method name if the classname is short
      size_t sparesForMethodName(0);

      // Get the string length for both segments
      size_t classNameLength(strlen(className));
      size_t methodNameLength(strlen(m_function));

      // Give extra to the function name
      if ( classNameLength < MAX_CLASSNAME_REPR )
      {
         sparesForMethodName = MAX_CLASSNAME_REPR - classNameLength;
      }

      // Give the extra to the class name
      if ( methodNameLength < MAX_METHODNAME_REPR )
      {
         sparesForClassName = MAX_METHODNAME_REPR - methodNameLength;
      }


      p[eCLASSNAME]=log_get_truncated_string(
         className, MAX_CLASSNAME_REPR+sparesForClassName,
         &p[eCLASSNAME_TRUNCATION] );

      p[eDOUBLE_COLON_SEPARATOR]=DOUBLE_COLON;

      // Is the method a dtor?
      int dtorMarkerLength(0);

      // If the '~' cropped out from the name?
      if ( m_function[0] == '~' && methodNameLength >= MAX_METHODNAME_REPR )
      {
         dtorMarkerLength=1;
         p[eDESTRUCTOR_TILDE]="~";
      }

      p[eMETHODNAME]=log_get_truncated_string(
         m_function, MAX_METHODNAME_REPR+sparesForMethodName-dtorMarkerLength,
         &p[eMETHODNAME_TRUNCATION] );

      snprintf( thisBuffer, sizeof(thisBuffer), THIS_MARKER, m_this );
      p[eEND] = thisBuffer;
   }
   else
   {
      // Use up all the repr space for the function name
      p[eMETHODNAME]=log_get_truncated_string(
         m_function, _LOG_MAX_FUNCTION_REPR_LENGTH, &p[eMETHODNAME_TRUNCATION] );
   }

   //
   // Change the built in variables
   //
   _log_line     = m_line;
   _log_filename = m_file;

   // Concat into single string
   static char fullFunctionName[_LOG_SIZED_FOR(_LOG_MAX_FUNCTION_REPR_LENGTH)];
   char *pTo = fullFunctionName;

   for ( int i=0; i<=eEND; ++i )
   {
      const char *pFrom = p[i];

      while ( pFrom && *pFrom )
      {
         *pTo++ = *pFrom++;
      }
   }

   // Terminate
   *pTo = '\0';

   // Force new function name
   _log_function = fullFunctionName;

   // Dump on the screen
   _log_type = LOG_LEVEL_TRACE;
   _log_trace( m_mask, "%s", strInOrOut );

   // Reset old debug so that the function name appears clearly
   _log_reset_last_function();
}


/* ---------------------------- End of file ------------------------------- */
