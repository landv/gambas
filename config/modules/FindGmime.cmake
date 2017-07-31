
if(GMIME_INCLUDE_DIRS AND GMIME_LIBRARIES)
    set(GMIME_FIND_QUIETLY TRUE)
    return()
endif()

find_package(PkgConfig)
pkg_check_modules(GMIME gmime-2.6)

set(VERSION_OK TRUE)
if (GMIME_VERSION)
    if (GMIME_FIND_VERSION_EXACT)
        if (NOT("${GMIME_FIND_VERSION}" VERSION_EQUAL "${GMIME_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${GMIME_VERSION}" VERSION_LESS "${GMIME_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMIME DEFAULT_MSG GMIME_INCLUDE_DIRS GMIME_LIBRARIES VERSION_OK)
