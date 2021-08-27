##
##############################################################################
## Builds the command shared debug object and the Python shared object
##
## @author gpa
##############################################################################
##

BUILD_LOCAL_FIRST = 1

# Rules for building the standard log library

MAKE_LIB=static_logger


#
# Target specific options
#
ifdef m68k_elf_gcc
   logger.has_no_file=1
   logger.is_embedded=1
   CSRCS += logger_os_utasker.c
endif

ifdef arm_none_eabi
   logger.has_no_file=1
   logger.is_embedded=1
   
   # The implementation needs to provide all missing symbols
endif

ifdef sunos
   CXXSRCS += logger_os_sunos.cpp
   logger.CSRCS = logger_init_unix.c logger_trace_stack.cpp
endif

ifdef logger.threaded_log
   IMPORT_CPPFLAGS+=-DLOG_THREADED_LOG
endif

ifdef linux
   CXXSRCS += logger_os_linux.cpp logger_trace_stack_linux.cpp logger_init_unix.cpp

ifdef logger.threaded_log
   CXXSRCS += logger_os_linux_thread.cpp 
endif
endif

ifdef win32
   CXXSRCS += logger_win32.cpp
   logger.CSRCS = logger_init_win32.c logger_trace_stack.cpp
endif

ifdef logger.has_no_file
   IMPORT_CPPFLAGS+=-DLOGGER_HAS_NO_FILE_SUPPORT
endif

ifndef logger.is_embedded
   CXXSRCS += logger_cxx.cpp
   MAKE_SHARED=logger
else
   IMPORT_CPPFLAGS+=-DLOGGER_SMALL
endif

ifdef logger.disabled
   IMPORT_CPPFLAGS+=-DFORCE_NODEBUG
   CSRCS += logger_disabled.c
else
   CSRCS += logger_common.c
endif

run::


## ---------------------------- End of file ----------------------------------
