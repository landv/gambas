include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckCCompilerFlag)
include(CheckTypeSize)

set(CMAKE_REQUIRED_LIBRARIES "-lm")

macro(set_system_variables)
    
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(OS_GNU 1)
        set(OS_LINUX 1)
        set(SYSTEM "Linux")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        set(OS_BSD 1)
        set(OS_FREEBSD 1)
        set(SYSTEM "FreeBSD")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        set(OS_BSD 1)
        set(OS_OPENBSD 1)
        set(SYSTEM "OpenBSD")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
        set(OS_BSD 1)
        set(OS_NETBSD 1)
        set(SYSTEM "NetBSD")
    elseif(${CYGWIN})
        set(OS_CYGWIN 1)
        set(SYSTEM "Cygwin")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "(Darwin|Rhapsody)")
        set(OS_BSD 1)
        set(OS_FREEBSD 1)
        set(OS_MACOSX 1)
        set(SYSTEM "MacOSX")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "k*BSD")
        set(OS_GNU 1)
        set(OS_BSD 1)
        set(OS_KFREEBSD 1)
        set(SYSTEM "kFreeBSD")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Solaris")
        set(OS_SOLARIS 1)
        set(SYSTEM "Solaris")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "GNU")
        set(OS_GNU 1)
        set(OS_HURD 1)
        set(SYSTEM "Hurd")
    else()
        message(WARNING "Unsupported system : ${CMAKE_SYSTEM_NAME}")
        set(SYSTEM "Unknown")
    endif()
    
endmacro()

#i*86-*-* )
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^i.86$")
    set(ARCHITECTURE x86)
    set(ARCH_X86 1)
#x86_64-*-* | amd64-* | ia64-*
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(x86_64|amd64|ia64)$")
    set(ARCHITECTURE x86_64)
    set(ARCH_X86_64 1)
#arm*-*-* )
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm")
    set(ARCHITECTURE arm)
    set(ARCH_ARM 1)
#powerpc-*-* )
elseif(${CMAKE_SYSTEM_PROCESSOR} EQUAL "powerpc")
    set(ARCHITECTURE powerpc)
    set(ARCH_PPC 1)
else()
    set(ARCHITECTURE unknown)
    message("Architecture '${CMAKE_SYSTEM_PROCESSOR}' is unknown")
endif()

#We do not use libtool to load shared libraries anymore!
set(DONT_USE_LTDL 1)

set(GAMBAS_FULL_VERSION 0x03100090)
set(GAMBAS_PCODE_VERSION 0x03100000)
set(GAMBAS_PCODE_VERSION_MIN 0x03000000)

# GIT_REVISION # git rev-parse --short HEAD
if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/.git")
    execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("-- Found Git working tree at revision ${GIT_REVISION}")
endif()

check_function_exists(atoll HAVE_ATOLL)
check_function_exists(cfmakeraw HAVE_CFMAKERAW)

#Have libiconv
check_include_file(dirent.h HAVE_DIRENT_H)
check_function_exists(daemon HAVE_DAEMON)

check_include_file(dlfcn.h HAVE_DLFCN_H)
check_library_exists(m exp10 "math.h" HAVE_EXP10)
check_library_exists(m exp2 "math.h"  HAVE_EXP2)

#Have libffi 
include("FindLibFFI" OPTIONAL RESULT_VARIABLE FOUND_PACKAGE)
    
if(NOT ${FOUND_PACKAGE} STREQUAL "NOTFOUND")
    find_package("LibFFI")
    if(${LIBFFI_FOUND})
        set(HAVE_FFI_COMPONENT 1)
    endif()
endif()


check_include_file(fcntl.h HAVE_FCNTL_H)
check_function_exists(gai_strerror HAVE_GAI_STRERROR)
#HAVE_GCC_VISIBILITY
check_c_compiler_flag("-fvisibility=hidden" HAVE_GCC_VISIBILITY)
check_function_exists(getaddrinfo HAVE_GETADDRINFO)
check_function_exists(getdomainname HAVE_GETDOMAINNAME)
check_function_exists(getnameinfo HAVE_GETNAMEINFO)
check_function_exists(getpagesize HAVE_GETPAGESIZE)
check_function_exists(getpt HAVE_GETPT)
check_include_file(grp.h HAVE_GRP_H)
#have gettextlib
#have gb.httpd
#have gb.inotify
#int64_t exists
check_type_size(int64_t HAVE_INT64T)
#have libintl
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_function_exists(kqueue HAVE_KQUEUE)
#cmakedefine HAVE_LIBCRYPT
#cmakedefine HAVE_LIBINET6
check_library_exists(m log2 "math.h"  HAVE_LOG2)
check_include_file(memory.h HAVE_MEMORY_H)
check_function_exists(mmap HAVE_MMAP)
check_include_file(ndir.h HAVE_NDIR_H)
check_include_file(osreldate.h HAVE_OSRELDATE_H)
check_include_file(paths.h HAVE_PATHS_H)
check_function_exists(poll HAVE_POLL)
check_include_file(poll.h HAVE_POLL_H)
check_function_exists(select HAVE_SELECT)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(setlogin HAVE_SETLOGIN)
check_function_exists(setsid HAVE_SETSID)
#cmakedefine HAVE_SOCKLENT
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h")
check_type_size(socklen_t HAVE_SOCKLENT)
set(CMAKE_EXTRA_INCLUDE_FILES "")

check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_function_exists(strerror HAVE_STRERROR)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/devpoll.h HAVE_SYS_DEVPOLL_H)
check_include_file(sys/dir.h HAVE_SYS_DIR_H)
check_include_file(sys/event.h HAVE_SYS_EVENT_H)
check_include_file(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_file(sys/param.h HAVE_SYS_PARAM_H)
check_include_file(sys/poll.h HAVE_SYS_POLL_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
#cmakedefine HAVE_TM_GMTOFF
check_include_file(sys/unistd.h HAVE_UNISTD_H)
#cmakedefine HAVE_UNISTD_H
check_function_exists(unsetenv HAVE_UNSETENV)
check_function_exists(vsnprintf HAVE_VSNPRINTF)
check_function_exists(waitpid HAVE_WAITPID)
#cmakedefine HAVE__PROGNAME

#TODO: Better packaging integration
set(PACKAGE "gambas3-main")

string(SUBSTRING "${CMAKE_SHARED_LIBRARY_SUFFIX}" 1 -1 SHARED_LIBRARY_EXT) #Remove the "." in the suffix

set(VERSION "${GAMBAS_VERSION_MAJOR}.${GAMBAS_VERSION_MINOR}.${GAMBAS_VERSION_REVISON}")

set_system_variables()

configure_file(config/config.h.in config/config.h)

set(CMAKE_REQUIRED_LIBRARIES "")
