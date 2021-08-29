#
#-----------------------------------------------------------------------------
# Tracing library to help debugging of Py, C, C++ applications
#-----------------------------------------------------------------------------
#

# PUBLIC & PRIVATE INCLUDES
##############################################################################
logger.PUBLIC_INCLUDES = include

# PUBLIC INTERFACES
##############################################################################
logger.PUBLIC_PYTHON   = $(logger.COMP_PATH)$(EXPORT_BASE)/$(LIBDIR)

# REQUIRED COMPONENTS
##############################################################################

SVNSERVER=$(svnServer_tools)
SVNSELECTOR=$(svnSelector_tools)
logger.REQUIRES = tools/pactMake


# ------------------------------   End of file   ------------------------------
