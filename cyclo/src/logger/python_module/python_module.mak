##
##############################################################################
## Builds the command shared debug object and the Python shared object
## @author gpa
## Copyright (c) Acterna, Plymouth 2000
##############################################################################
##


# Override default shared name
LibSharedPattern = $(BUILDDIR)/%.so

# Force Python headers and libs in case the make does not
USING_PYTHON_DEV=1

# Rules for building the standard log library
MAKE_SHARED=LOG

$(MAKE_SHARED).CSRCS = pyLogger.c
$(MAKE_SHARED).PACKAGES = logger
$(MAKE_SHARED).INSTALLDIR = $(DEFAULT_PYTHON_INSTALLDIR)/lib
$(MAKE_SHARED).EXT_LIBS += $(PYTHON_DEV_LIB)

# Required to link against the Python library
EXT_LIB_PATHS += $(PYTHON_DEV_LIB_PATH)

# Allow embedding the logger
MAKE_LIB=pylog

# Same source as the python module
$(MAKE_LIB).CSRCS = pyLogger.c

# Generate a runpy for convenience
all:: runpy


## ---------------------------- End of file ----------------------------------
