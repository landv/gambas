# - Find pcre
# Find the native PCRE includes and library
#
#  PCRE_INCLUDE_DIR  - where to find pcre.h, etc.
#  PCRE_LIBRARY      - Path to the pcre library.
#  PCRE_FOUND        - True if pcre found.

IF (PCRE_INCLUDE_DIR)
  # Already in cache, be silent
  SET(PCRE_FIND_QUIETLY TRUE)
ENDIF (PCRE_INCLUDE_DIR)

FIND_PATH(PCRE_INCLUDE_DIR pcre.h)

SET(PCRE_NAMES pcre)
FIND_LIBRARY(PCRE_LIBRARY NAMES ${PCRE_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

SET(PCRE_INCLUDE_DIR ${SYSTEM_PCRE_INCLUDE_DIR} CACHE PATH "PCRE include directory")
SET(PCRE_LIBRARY ${SYSTEM_PCRE_LIBRARY} CACHE FILEPATH "PCRE library")

MARK_AS_ADVANCED( PCRE_LIBRARY PCRE_INCLUDE_DIR )
