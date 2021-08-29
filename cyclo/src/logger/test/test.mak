##
##############################################################################
## Test the tracing library.
## Should not be included by parent directory - hence making bin
##
## @author gax
##############################################################################
##

# Rules for building the test harness
MAKE_BIN=testLog testSmall testOff

testLog.CXXSRCS = test_cplusplus.cpp
testLog.CSRCS = test_C_file_with_a_very_long_file_name.c test_C.c
testLog.PACKAGES += logger

embedded.CSRCS = logger_common.c logger_os_minimalist.c
testSmall.CSRCS += $(embedded.CSRCS) $(testLog.CSRCS) test_embedded.c

testSmall.IMPORT_CPPFLAGS+=-DLOGGER_HAS_NO_FILE_SUPPORT -DLOGGER_SMALL

testOff.CSRCS = testForceNoDebug.c
testOff.IMPORT_CPPFLAGS+=$(testSmall.IMPORT_CPPFLAGS) -DFORCE_NODEBUG
testOff.PACKAGES = static_logger

# Compile the embedded source even in Linux/Windows
C_VPATH=../src

ifeq ($(USING_PYTHON_DEV),1)
MAKE_PYTHON = pylog

pylog.PYSRCS = test_config.py test_basic_python.py test_python.py

MAKE_SHARED = combined

combined.CXXSRCS = test_combined.cpp
combined.SWIGSRCS = test_combined.i
combined.PACKAGES = logger
endif

# Rule for running the regression test harmess
run::
	./test_regression_harness.bash $(BUILDDIR)

clean::
	@$(SMUTE); $(RM) test_results_raw.txt test_results.txt other_test_results.txt

# Make sure the DFORCE_NODEBUG does not reference any log functions
run::
	@$(ECHO) Make sure the FORCE_NODEBUG does not reference any log functions
	@$(SMUTE); \
	  count=$$(nm  ./x86_64-linux-gnu_debug/testOff | grep '_log' | wc -l); \
	  ((count)) && echo FAILED || echo OK

## ---------------------------- End of file ----------------------------------

