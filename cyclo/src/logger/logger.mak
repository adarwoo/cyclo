##
##############################################################################
## Author:      gax
## Description: Top makefile for making the log component
##############################################################################
##


##############################################################################
#   Sub-directories, sources files and object files definiton section
#
SUBDIRS=src

# The test is only for a x86 platform
ifneq ($(findstring x86, $(platform)),)
SUBDIRS+=test
endif

# The logger python is only for platforms supporting python
ifdef USING_PYTHON_DEV
SUBDIRS+=python_module
endif
