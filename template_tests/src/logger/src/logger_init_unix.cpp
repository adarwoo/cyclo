/*-
 *****************************************************************************
 * Force the library initialisation
 * The __attribute ((constructor)) could end up as a no-operation if
 *  invoked inside C++ code. This file make this a statement.
 *
 * @author gpa
 *****************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <cstdlib>
#include <exception>

extern "C" void _log_init( bool splitPatternOff );
extern "C" void _log_exit( void );
extern "C" void _log_trace( const char *mask, const char *Format, ...);
extern "C" void _log_trace_error( const char *mask, const char *str );
extern "C" void _log_trace_stack();

/**
 *****************************************************************************
 * Force DSO initialisation
 *
 *****************************************************************************
 */

#ifdef LOGGER_HAS_EXCEPTIONS
/**
 * Overrides the standard terminate with this version which dumps the stack
 *  in the logger
 */
namespace
{
   void logger_terminate()
   {
      try
      {
         throw;
      }
      catch( const std::exception& x )
      {
         _log_trace_error(0, "Uncaught standard (or derived) exception cause a termination.");
         _log_trace_error(0, "what() is:");
         _log_trace_error(0, x.what());
      }
      catch (...)
      {
         _log_trace_error(0, "Uncaught exception (non-std::exception) caused a termination");
      }

      _log_trace_stack();

      abort();
   }
}
#endif 

/**
 * Signal handler for SIGSEGV signals
 *
 * @param[in] signum Signal number
 */
extern "C"
{
   static void segv_handler(int signum)
   {
      // Segfault catched: log stack
      printf("Process %d got signal %d (%s)\n", getpid(), signum, strsignal(signum));

      // Restore default SIGSEGV handler: next SIGSEGV will be handled by this default one
      // => A core will be generated if enabled and debugger will be able to catch signal
      // Note: gdb may catch the first signal
      signal(signum, SIG_DFL);

      // Backtrace
      _log_trace_stack();

      // Emit signal again for cases when segfault condition disappeared or external SIGSEGV signal
      kill(getpid(), signum);
   }
}


void __attribute ((constructor)) init_function (void)
{
   _log_init( false );

   // Register signal handler for SIGSEGV in order to log backtrace AND provide a core dump
   // Took idea from http://www.alexonlinux.com/how-to-handle-sigsegv-but-also-generate-core-dump
   signal(SIGSEGV, segv_handler);

#ifdef LOGGER_HAS_EXCEPTIONS
   // Install our terminate handler
   std::set_terminate( logger_terminate );
#endif
}


/* ---------------------------- End of file ------------------------------- */

