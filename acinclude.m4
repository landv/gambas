#######################################################################################
##
##   The following macros are specific to Gambas.
##   Some of them are made by me (Benoît Minisini)
##   Feel free to use these macros as you need !
##
##   IMPORTANT: This file is shared by all Gambas
##   source packages
##
#######################################################################################

## ---------------------------------------------------------------------------
## GB_MESSAGE
## Prints a message, and stores it in a summay file to print it later
## ---------------------------------------------------------------------------

AC_DEFUN([GB_MESSAGE],
[
  echo "|| $1" >> $srcdir/warnings.log
])

## ---------------------------------------------------------------------------
## GB_MESSAGE
## Prints a warning message, and stores it in a summay file to print it later
## ---------------------------------------------------------------------------

AC_DEFUN([GB_WARNING],
[
  AC_MSG_WARN($1)
  GB_MESSAGE([$1])
])

## ---------------------------------------------------------------------------
## GB_CLEAR_MESSAGES
## Clear summary
## ---------------------------------------------------------------------------

AC_DEFUN([GB_CLEAR_MESSAGES],
[
  rm -f $srcdir/warnings.log
  touch $srcdir/warnings.log
])

## ---------------------------------------------------------------------------
## GB_PRINT_MESSAGES
## Print summary
## ---------------------------------------------------------------------------

AC_DEFUN([GB_PRINT_MESSAGES],
[
  if test -s $srcdir/warnings.log; then
    echo
    echo "||"
    cat $srcdir/warnings.log
    echo "||"
    echo
  fi
])

## ---------------------------------------------------------------------------
## GB_INIT_AUTOMAKE
## automake initialization with common version number
## ---------------------------------------------------------------------------

AC_DEFUN([GB_INIT_AUTOMAKE],
[
  AM_INIT_AUTOMAKE([subdir-objects])
  m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])
  AC_CONFIG_HEADER([config.h])
  AC_PREFIX_DEFAULT(/usr)

  GAMBAS_VERSION=GB_VERSION_MAJOR
  GAMBAS_MINOR_VERSION=GB_VERSION_MINOR

  AC_SUBST(GAMBAS_VERSION)
  AC_SUBST(GAMBAS_MINOR_VERSION)

  AC_DEFINE(GAMBAS_VERSION, GB_VERSION_MAJOR, Gambas version)
  AC_DEFINE(GAMBAS_MINOR_VERSION, GB_VERSION_MINOR, Gambas minor version)

  AC_DEFINE(GAMBAS_VERSION_STRING, "GB_VERSION_MAJOR", Gambas version string)
  AC_DEFINE(GAMBAS_FULL_VERSION_STRING, "GB_VERSION_MAJOR.GB_VERSION_MINOR", Gambas full version string)

  AC_DEFINE(GAMBAS_FULL_VERSION, 0x03060090, [Full Gambas version])
  AC_DEFINE(GAMBAS_PCODE_VERSION, 0x03060090, [Gambas bytecode version])
  AC_DEFINE(GAMBAS_PCODE_VERSION_MIN, 0x03000000, [Minimum Gambas bytecode version])

  GB_CLEAR_MESSAGES
])

## ---------------------------------------------------------------------------
## GB_CONFIG_SUBDIRS
## configuration of a component sub-directory, with a flag for disabling it
## ---------------------------------------------------------------------------

AC_DEFUN([GB_CONFIG_SUBDIRS],
[
  AC_ARG_ENABLE(
    $1,
    [  --enable-$1                enable $1 component (default: yes)],
    gb_enable_$1=$enableval,
    gb_enable_$1=yes
  )

  if test "$gb_enable_$1" = "yes"; then
    if test -d $srcdir/$2; then
      AC_CONFIG_SUBDIRS($2)
      $1_dir=$2
    fi
  else
    GB_WARNING([$1 component is disabled by configure option])
    $1_dir=""
  fi
  
  AC_SUBST($1_dir)
])

## ---------------------------------------------------------------------------
## GB_INIT_SHORT GB_INIT GB_LIBTOOL
## configure.ac initialization
## ---------------------------------------------------------------------------

AC_DEFUN([GB_INIT_SHORT],
[
  AC_CONFIG_SRCDIR([configure.ac])
  AM_MAINTAINER_MODE

  COMPONENT=$1

  GB_INIT_AUTOMAKE

  AC_CANONICAL_HOST
  
  gbbindir=$bindir/gambas$GAMBAS_VERSION
  AC_SUBST(gbbindir)
  gblibdir=$libdir/gambas$GAMBAS_VERSION
  AC_SUBST(gblibdir)
  gbdatadir=$datadir/gambas$GAMBAS_VERSION
  AC_SUBST(gbdatadir)

  AC_PROG_INSTALL
  AC_PROG_LN_S
])

AC_DEFUN([GB_LIBTOOL],
[
  AC_LIBTOOL_DLOPEN
  AC_LIBLTDL_CONVENIENCE
  AC_LIBTOOL_WIN32_DLL
  AC_DISABLE_STATIC

  AC_SUBST(INCLTDL)
  AC_SUBST(LIBLTDL)

  dnl LD_FLAGS="-Wl,-O1"
  if test $SYSTEM == "CYGWIN"; then
    LD_FLAGS="$LD_FLAGS -no-undefined"
  fi
  AC_SUBST(LD_FLAGS)
])

AC_DEFUN([GB_INIT],
[
  GB_INIT_SHORT($1)
  GB_SYSTEM
  GB_LIBTOOL
  
  dnl ---- Checks for programs

  AC_PROG_CPP
  AC_PROG_CXX
  AC_PROG_CC
  AC_PROG_MAKE_SET

  dnl ---- Checks for header files.

  dnl AC_HEADER_DIRENT
  dnl AC_HEADER_STDC
  dnl AC_HEADER_SYS_WAIT
  dnl AC_CHECK_HEADERS(fcntl.h limits.h malloc.h strings.h sys/ioctl.h sys/time.h unistd.h)

  dnl ---- Checks for typedefs, structures, and compiler characteristics.

  dnl AC_C_CONST
  dnl AC_TYPE_PID_T
  dnl AC_TYPE_SIZE_T
  dnl AC_HEADER_TIME
  dnl AC_STRUCT_TM
  dnl AC_C_LONG_DOUBLE

  dnl ---- Checks for library functions.

  dnl AC_FUNC_ALLOCA
  dnl AC_PROG_GCC_TRADITIONAL
  dnl AC_TYPE_SIGNAL
  dnl AC_FUNC_STRCOLL
  dnl AC_FUNC_STRFTIME
  dnl AC_FUNC_VPRINTF
  dnl AC_FUNC_WAIT3
  dnl AC_CHECK_FUNCS(getcwd gettimeofday mkdir rmdir select socket strdup strerror strtod strtol sysinfo)
  
  AC_CHECK_FUNCS(setenv unsetenv getdomainname getpt cfmakeraw)

  dnl ---- Checks for libraries

  dnl AC_CHECK_LIB(m, main, echo)
  dnl AC_CHECK_LIB(z, main, echo)

  C_LIB=-lc

  AC_SUBST(C_LIB)

  AC_CHECK_LIB(gcc_s, main, CXX_LIB="$CXX_LIB -lgcc_s")
  AC_CHECK_LIB(stdc++, main, CXX_LIB="$CXX_LIB -lstdc++")

  AC_SUBST(CXX_LIB)

  dnl ---- Check for shared library extension
  
  GB_SHARED_LIBRARY_EXT()
  
  dnl ---- Check for threading
  
  GB_THREAD()
  
  dnl ---- Check for mathematic libraries
  
  GB_MATH()
  
  dnl ---- Check for gettext lib
  
  GB_GETTEXT()

  dnl ---- Support for colorgcc
  dnl ---- WARNING: libtool does not support colorgcc!

  dnl AC_PATH_PROG(COLORGCC, colorgcc)

  if test x"$COLORGCC" != x; then
    if test "$gambas_colorgcc" = "yes"; then
      CC="colorgcc"
      CXX="g++"
    fi
  fi
  
  dnl ---- Support for ccache
  
  AC_ARG_ENABLE(
    ccache,
    [  --enable-ccache                use ccache if present (default: yes)],
    gambas_ccache=$enableval,
    gambas_ccache=yes
  )

  AC_PATH_PROG(CCACHE, ccache)

  if test "$gambas_colorgcc" = "yes"; then
    if test x"$CCACHE" != x; then
      
      CC="ccache $CC"
      CXX="ccache $CXX"
      
      if test x"$COLORGCC" != x; then
        if test "$gambas_colorgcc" = "yes"; then
          CC="colorgcc"
          CXX="colorgcc"
        fi
      fi
    
    fi
  fi

  dnl ---- debug option

  AC_ARG_ENABLE(
    debug,
    [  --enable-debug                 compile for debugging (default: yes)],
    gambas_debug=$enableval,
    gambas_debug=yes
  )

  AM_CONDITIONAL(DEBUG, test "$gambas_debug" = yes)

  dnl ---- optimization option

  AC_ARG_ENABLE(
    optimization,
    [  --enable-optimization          compile with optimizations (default: yes)],
    gambas_optimization=$enableval,
    gambas_optimization=yes
  )

  AM_CONDITIONAL(OPTIMIZE, test "$gambas_optimization" = yes)

  AM_CFLAGS="$AM_CFLAGS -pipe -Wall -Wno-unused-value -fsigned-char"
  if test $SYSTEM = "MACOSX"; then
    AM_CFLAGS="$AM_CFLAGS -fnested-functions"
  fi

  AM_CXXFLAGS="$AM_CXXFLAGS -pipe -Wall -fno-exceptions -Wno-unused-value -fsigned-char"

  dnl ---- Check for gcc visibility flag
  
  have_gcc_visibility=no
  
  if test $SYSTEM != "CYGWIN"; then
    GB_CFLAGS_GCC_OPTION([-fvisibility=hidden],,
      [
      AM_CFLAGS="$AM_CFLAGS -fvisibility=hidden"
      AM_CXXFLAGS="$AM_CXXFLAGS -fvisibility=hidden"
      have_gcc_visibility=yes])
  fi
	
  if test "$have_gcc_visibility" = "yes"; then
    AC_DEFINE(HAVE_GCC_VISIBILITY, 1, [Whether gcc supports -fvisibility=hidden])
  fi

  dnl ---- Debug flags
  
  if test "$gambas_debug" = "yes"; then
    AM_CFLAGS="$AM_CFLAGS -g -ggdb"
    AM_CXXFLAGS="$AM_CXXFLAGS -g -ggdb"
  fi

  dnl ---- Optimization flags
  
  if test "x$gambas_optimization" = "xyes"; then
    AM_CFLAGS_OPT="$AM_CFLAGS -O3"
    AM_CFLAGS="$AM_CFLAGS -Os"
    AM_CXXFLAGS_OPT="$AM_CXXFLAGS -O3 -fno-omit-frame-pointer"
    AM_CXXFLAGS="$AM_CXXFLAGS -Os -fno-omit-frame-pointer"
  else
    AM_CFLAGS_OPT="$AM_CFLAGS -O0"
    AM_CFLAGS="$AM_CFLAGS -O0"
    AM_CXXFLAGS_OPT="$AM_CXXFLAGS -O0"
    AM_CXXFLAGS="$AM_CXXFLAGS -O0"
  fi

  CFLAGS=""
  CXXFLAGS=""
  
  AC_SUBST(AM_CFLAGS)
  AC_SUBST(AM_CFLAGS_OPT)
  AC_SUBST(AM_CXXFLAGS)
  AC_SUBST(AM_CXXFLAGS_OPT)
  
  rm -f DISABLED DISABLED.*
])


## ---------------------------------------------------------------------------
## GB_THREAD
## Detect threading compiler options
## ---------------------------------------------------------------------------

AC_DEFUN([GB_THREAD],
[
  case "${host}" in
    *-*-freebsd* | *-*-darwin* )
      THREAD_LIB=""
      THREAD_INC="-pthread -D_REENTRANT"
      GBX_THREAD_LIB=""
      GBX_THREAD_INC="-pthread -D_REENTRANT"
      ;;
    *-*-netbsd* )
      THREAD_LIB=""
      THREAD_INC="-pthread -D_REENTRANT"
      GBX_THREAD_LIB=""
      GBX_THREAD_INC="-pthread -D_REENTRANT"
      ;;
    *)
      THREAD_LIB="-lpthread"
      THREAD_INC="-D_REENTRANT"
      GBX_THREAD_LIB="-lpthread"
      GBX_THREAD_INC="-D_REENTRANT"
      ;;
  esac

  AC_MSG_CHECKING(for threading compiler options)
  AC_MSG_RESULT($THREAD_INC)
  AC_MSG_CHECKING(for threading linker options)
  AC_MSG_RESULT($THREAD_LIB)

  AC_SUBST(THREAD_LIB)
  AC_SUBST(THREAD_INC)
  AC_SUBST(GBX_THREAD_LIB)
  AC_SUBST(GBX_THREAD_INC)
])


## ---------------------------------------------------------------------------
## GB_MATH
## Detect mathematic libraries
## ---------------------------------------------------------------------------

AC_DEFUN([GB_MATH],
[
  case "${host}" in
    *-*-freebsd* )
      MATH_LIB="-lm"
      ;;
    *)
      MATH_LIB="-lm"
      ;;
  esac

  AC_MSG_CHECKING(for mathematic libraries)
  AC_MSG_RESULT($MATH_LIB)

  AC_SUBST(MATH_LIB)
])


## ---------------------------------------------------------------------------
## GB_CHECK_MATH_FUNC
## Check a specific mathematical function
##
##   $1 = name of the function
##   $2 = macro to define
## ---------------------------------------------------------------------------

AC_DEFUN([GB_CHECK_MATH_FUNC],
[AC_CACHE_CHECK(for $1,
  gb_cv_math_$1,
[AC_TRY_COMPILE([
#define _ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define __USE_ISOC99	1
#define __USE_ISOC9X	1
#include <math.h>],
[	int value = $1 (1.0) ; ], gb_cv_math_$1=yes, gb_cv_math_$1=no)])
if test $gb_cv_math_$1 = yes; then
  AC_DEFINE(HAVE_$2, 1,
            [Define if you have $1 function.])
fi
])

## ---------------------------------------------------------------------------
## GB_MATH_FUNC
## Detect which mathematical functions are available
## ---------------------------------------------------------------------------

AC_DEFUN([GB_MATH_FUNC],
[
  ac_save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS -$MATH_LIB"

  GB_CHECK_MATH_FUNC(exp10, EXP10)
  GB_CHECK_MATH_FUNC(exp2, EXP2)
  GB_CHECK_MATH_FUNC(log2, LOG2)

  LDFLAGS=$ac_save_LDFLAGS
])


## ---------------------------------------------------------------------------
## GB_SYSTEM
## Detects the target system and its architecture
## ---------------------------------------------------------------------------

AC_DEFUN([GB_SYSTEM],
[
  AC_MSG_CHECKING(target system)

  case "${host}" in
    *-*-linux*-gnu* )
      SYSTEM=LINUX
      AC_DEFINE(OS_GNU, 1, [Target system is of GNU family])
      AC_DEFINE(OS_LINUX, 1, [Target system is Linux])
      AC_DEFINE(SYSTEM, "Linux", [Operating system])
      ;;
    *-*-freebsd* )
      SYSTEM=FREEBSD
      AC_DEFINE(OS_BSD, 1, [Target system is of BSD family])
      AC_DEFINE(OS_FREEBSD, 1, [Target system is FreeBSD])
      AC_DEFINE(SYSTEM, "FreeBSD", [Operating system])
      ;;
    *-*-netbsd* )
      SYSTEM=NETBSD
      AC_DEFINE(OS_BSD, 1, [Target system is of BSD family])
      AC_DEFINE(OS_NETBSD, 1, [Target system is NetBSD])
      AC_DEFINE(SYSTEM, "NetBSD", [Operating system])
      ;;
    *-*-openbsd* )
      SYSTEM=OPENBSD
      AC_DEFINE(OS_BSD, 1, [Target system is of BSD family])
      AC_DEFINE(OS_OPENBSD, 1, [Target system is OpenBSD])
      AC_DEFINE(SYSTEM, "OpenBSD", [Operating system])
      ;;
    *-*-cygwin* )
      SYSTEM=CYGWIN
      AC_DEFINE(OS_CYGWIN, 1, [Target system is Cygwin/Windows])
      AC_DEFINE(SYSTEM, "Cygwin", [Operating system])
      ;;
    *-*-darwin* | *-*-rhapsody* )
      SYSTEM=MACOSX
      AC_DEFINE(OS_BSD, 1, [Target system is of BSD family])
      AC_DEFINE(OS_FREEBSD, 1, [Target system is FreeBSD])
      AC_DEFINE(OS_MACOSX, 1, [Target system is MacOS X])
      AC_DEFINE(SYSTEM, "MacOSX", [Operating system])
      ;;
    *-*-solaris* )
      SYSTEM=SOLARIS
      AC_DEFINE(OS_SOLARIS, 1, [Target system is Solaris])
      AC_DEFINE(SYSTEM, "Solaris", [Operating system])
      ;;
    *-*-k*bsd*-gnu* )
      SYSTEM=KFREEBSD
      AC_DEFINE(OS_BSD, 1, [Target system is of BSD family])
      AC_DEFINE(OS_GNU, 1, [Target system is of GNU family])
      AC_DEFINE(OS_KFREEBSD, 1, [Target system is kFREEBSD])
      AC_DEFINE(SYSTEM, "kFreeBSD", [Operating system])
      ;;
    *-gnu* )
      SYSTEM=HURD
      AC_DEFINE(OS_GNU, 1, [Target system is of GNU family])
      AC_DEFINE(OS_HURD, 1, [Target system is Hurd])
      AC_DEFINE(SYSTEM, "Hurd", [Operating system])
      ;;
    * )
      SYSTEM=UNKNOWN
      AC_DEFINE(SYSTEM, "unknown", [Operating system])
      GB_MESSAGE([System is unknown])
      ;;
  esac

  AC_MSG_RESULT($SYSTEM)

  AC_MSG_CHECKING(target architecture)

  case "${host}" in
    i*86-*-* )
      ARCH=X86
      AC_DEFINE(ARCH_X86, 1, [Target architecture is x86])
      AC_DEFINE(ARCHITECTURE, "x86", [Architecture])
      ;;
    x86_64-*-* | amd64-* | ia64-* )
      ARCH=X86_64
      AC_DEFINE(ARCH_X86_64, 1, [Target architecture is x86_64])
      AC_DEFINE(ARCHITECTURE, "x86_64", [Architecture])
      ;;
    arm*-*-* )
      ARCH=ARM
      AC_DEFINE(ARCH_ARM, 1, [Target architecture is ARM])
      AC_DEFINE(ARCHITECTURE, "arm", [Architecture])
      ;;
    powerpc-*-* )
      ARCH=PPC
      AC_DEFINE(ARCH_PPC, 1, [Target architecture is PowerPC])
      AC_DEFINE(ARCHITECTURE, "powerpc", [Architecture])
      ;;
    *)
      ARCH=UNKNOWN
      AC_DEFINE(ARCHITECTURE, "unknown", [Architecture])
      GB_MESSAGE([Architecture is unknown])
      ;;
  esac

  AC_MSG_RESULT($ARCH)
])


## ---------------------------------------------------------------------------
## GB_SHARED_LIBRARY_EXT
## Detects shared library extension
## ---------------------------------------------------------------------------

AC_DEFUN([GB_SHARED_LIBRARY_EXT],
[
  AC_MSG_CHECKING(which extension is used for shared libraries)

  case "${host}" in
    *-*-cygwin* )
      SHLIBEXT="la"
      AC_DEFINE(SHARED_LIBRARY_EXT, "dll", [Shared library extension is '.dll'])
      ;;
    *-*-darwin* )
      SHLIBEXT="dylib"
      AC_DEFINE(SHARED_LIBRARY_EXT, "dylib", [Shared library extension is '.dylib'])
      ;;
    *)
      SHLIBEXT="so"
      AC_DEFINE(SHARED_LIBRARY_EXT, "so", [Shared library extension is '.so'])
      ;;
  esac

  AC_SUBST(SHLIBEXT)

  AC_MSG_RESULT([.$SHLIBEXT])
])


## ---------------------------------------------------------------------------
## GB_GETTEXT
## Detects if we must link to an external gettext library
## ---------------------------------------------------------------------------

AC_DEFUN([GB_GETTEXT],
[
  AC_MSG_CHECKING(for external gettext library)

  case "${host}" in
    *-*-openbsd* )
      GETTEXT_LIB=-llibgettext
      ;;
    *)
      GETTEXT_LIB=
      ;;
  esac

  AC_SUBST(GETTEXT_LIB)

  AC_MSG_RESULT($GETTEXT_LIB)
])


## ---------------------------------------------------------------------------
## GB_FIND
## Find files in directories
##
##   $1 = Files to search
##   $2 = Directories
##   $3 = Sub-directories patterns
##
##   Returns a path list in $gb_val
## ---------------------------------------------------------------------------

AC_DEFUN([GB_FIND],
[
dnl echo "Searching $1, $2, $3"
gb_val=""
gb_save=`pwd`
gb_file_list="$1"

gb_main_dir_list="$2"
gb_sub_dir_list="$3"

gb_sub_dir_list_64=`echo "$gb_sub_dir_list" | sed s/"lib"/"lib64"/g`

## if there is 'lib' inside sub-directories, then we decide to search "lib64" first.

if test "$gb_sub_dir_list_64" != "$gb_sub_dir_list"; then
  gb_sub_dir_list="$gb_sub_dir_list_64 $gb_sub_dir_list";

  gb_main_dir_list_64=`echo "$gb_main_dir_list" | sed s/"lib"/"lib64"/g`

  if test "$gb_main_dir_list_64" != "$gb_main_dir_list"; then
    gb_main_dir_list="$gb_main_dir_list_64 $gb_main_dir_list";
  fi

fi

for gb_main_dir in $gb_main_dir_list; do
  dnl echo "search $gb_main_dir"
  if test -d $gb_main_dir; then
    cd $gb_main_dir
    for gb_search_dir in $gb_sub_dir_list; do
      for gb_dir in $gb_search_dir/ $gb_search_dir/*/ $gb_search_dir/*/*/ $gb_search_dir/*/*/*/; do

        dnl echo "search subdir $gb_dir"
        gb_new_file_list=""
        gb_find_dir=""

        for gb_file in $gb_file_list; do

          dnl echo "search file $gb_file"
          gb_find=no
          if test -r "$gb_main_dir/$gb_dir/$gb_file" || test -d "$gb_main_dir/$gb_dir/$gb_file"; then

            ifelse($4,[],

              gb_find=yes,

              for gb_test in $4; do
                gb_output=`ls -la $gb_main_dir/$gb_dir/$gb_file | grep "$gb_test"`
                if test "x$gb_output" != "x"; then
                  gb_find=yes
                fi
              done
            )

          fi

          if test "$gb_find" = "yes"; then
            dnl echo "FOUND!"
            if test "x$gb_find_dir" = "x"; then
              if test "x$gb_val" = "x"; then
                gb_val="$gb_main_dir/$gb_dir"
              else
                gb_val="$gb_val $gb_main_dir/$gb_dir"
              fi
            fi
            gb_find_dir=yes
          else
            gb_new_file_list="$gb_new_file_list $gb_file"
          fi

        done

        gb_file_list=$gb_new_file_list

        if test "x$gb_file_list" = "x " || test "x$gb_file_list" = "x"; then
          break 3
        fi

      done
    done
  fi
done

if test "x$gb_file_list" != "x " && test "x$gb_file_list" != "x"; then
  gb_val=no
fi

cd $gb_save
])


## ---------------------------------------------------------------------------
## GB_COMPONENT_PKG_CONFIG
## Component detection macro based on pkg-config
##
##   $1 = Component key in lower case (ex: pgsql)
##   $2 = Component key in upper case (ex: PGSQL)
##   $3 = Component name (ex: gb.db.postgresql)
##   $4 = Sub-directory name
##   $5 = pkg-config module(s) name(s) with optional required version(s)
##   $6 = Warning message (optional)
##
##   => defines HAVE_*_COMPONENT (to know if you can compile the component)
##      *_INC (for the compiler) and *_LIB / *_LDFLAGS (for the linker)
## ---------------------------------------------------------------------------

AC_DEFUN([GB_COMPONENT_PKG_CONFIG],
[
  AC_ARG_ENABLE(
    $1,
    [  --enable-$1                enable $3 (default: yes)],
    gb_enable_$1=$enableval,
    gb_enable_$1=yes
  )

  dnl AC_ARG_WITH($1-includes,
  dnl   [  --with-$1-includes      where the $3 headers are located. ],
  dnl   [  gb_inc_$1="$withval" ])

  dnl AC_ARG_WITH($1-libraries,
  dnl   [  --with-$1-libraries     where the $3 libraries are located. ],
  dnl   [  gb_lib_$1="$withval" ])

  have_$1=no
  
  if test "$gb_enable_$1"="yes" && test ! -e DISABLED && test ! -e DISABLED.$3; then

    AC_MSG_CHECKING(for $3 component with pkg-config)
  
    gb_inc_$1=""
    gb_lib_$1=""
    gb_ldflags_$1=""
    have_$1=yes
    gb_testval=""
        
    pkg-config --silence-errors --exists $5
    if test $? -eq "0"; then
      
      ## Checking for headers
      
      $2_INC="`pkg-config --cflags $5`"
    
      ## Checking for libraries
      
      $2_LIB="`pkg-config --libs-only-l $5`"
      $2_LDFLAGS="`pkg-config --libs-only-L $5` `pkg-config --libs-only-other $5`"
      $2_DIR=$4
          
    else
      
      have_$1=no
      
    fi

  fi

  if test "$have_$1" = "no"; then
  
    if test "$gb_in_component_search" != "yes"; then
      touch DISABLED
      touch DISABLED.$3
    fi

    AC_MSG_RESULT(no)

    for pkgcmp in $5
    do

      pkg-config --silence-errors --exists $pkgcmp
      if test $? -eq "1"; then
        GB_WARNING([Unable to met pkg-config requirement: $pkgcmp])
      fi

    done

  else
    
    AC_DEFINE(HAVE_$2_COMPONENT, 1, [Have $3 component])
    
    AC_MSG_RESULT(OK)
    
  fi
  
  if test "$have_$1" = "no"; then
  
    $2_INC=""
    $2_LIB=""
    $2_LDFLAGS=""
    $2_DIR=""
    if test "$gb_in_component_search" != "yes"; then
      if test x"$6" = x; then
	GB_WARNING([$3 is disabled])
      else
	GB_WARNING([$6])
      fi
    fi

  fi
  
  AC_SUBST($2_INC)
  AC_SUBST($2_LIB)
  AC_SUBST($2_LDFLAGS)
  AC_SUBST($2_DIR)
])


## ---------------------------------------------------------------------------
## GB_COMPONENT
## Component detection macro that searches for files
##
##   $1 = Component key in lower case (ex: postgresql)
##   $2 = Component key in upper case (ex: POSTGRESQL)
##   $3 = Component name (ex: gb.db.postgresql)
##   $4 = Sub-directory name
##   $5 = How to get include path (must return it in gb_val)
##   $6 = How to get library path (must return it in gb_val)
##   $7 = Libraries
##   $8 = Compiler flags (optional)
##   $9 = Warning message (optional)
##
##   => defines HAVE_*_COMPONENT (to know if you can compile the component)
##      *_INC (for the compiler) and *_LIB (for the linker)
## ---------------------------------------------------------------------------

AC_DEFUN([GB_COMPONENT],
[
  AC_ARG_ENABLE(
    $1,
    [  --enable-$1                enable $3 (default: yes)],
    gb_enable_$1=$enableval,
    gb_enable_$1=yes
  )

  gb_inc_$1=no
  gb_lib_$1=no

  if test "$gb_enable_$1" = "yes" && test ! -e DISABLED && test ! -e DISABLED.$3; then

    ## Checking for headers

    AC_MSG_CHECKING(for $3 headers)

    AC_ARG_WITH($1-includes,
      [  --with-$1-includes      where the $3 headers are located. ],
      [  gb_inc_$1="$withval" ])

    AC_CACHE_VAL(gb_cv_header_$1, [

      if test "$gb_inc_$1" = no; then
        gb_val=""
        $5
        gb_inc_$1=$gb_val
      fi

      gb_cv_header_$1=$gb_inc_$1
    ])

    AC_MSG_RESULT([$gb_cv_header_$1])
    
    if test "$gb_cv_header_$1" = "no"; then
      for gb_result in $gb_file_list; do
        GB_WARNING([Unable to find file: $gb_result])
      done
    fi

    $2_INC=""

    for gb_dir in $gb_cv_header_$1; do
      if test "$gb_dir" != "/usr/include"; then
        if test "$gb_dir" != "/usr/include/"; then
          $2_INC="$$2_INC -I$gb_dir"
        fi
      fi
    done

    if test "x$8" != "x"; then
      $2_INC="$$2_INC $8"
    fi

    if test "$gb_cv_header_$1" = no; then
      have_inc_$1="no"
      $2_INC=""
    else
      have_inc_$1="yes"
    fi

    ## Checking for libraries

    AC_MSG_CHECKING(for $3 libraries)

    AC_ARG_WITH($1-libraries,
      [  --with-$1-libraries     where the $3 libraries are located. ],
      [  gb_lib_$1="$withval" ])

    AC_CACHE_VAL(gb_cv_lib_$1, [

      if test "$gb_lib_$1" = no; then
        gb_val=""
        $6
        gb_lib_$1=$gb_val
      fi

      gb_cv_lib_$1=$gb_lib_$1
    ])

    if test "$gb_cv_lib_$1" = no; then
      have_lib_$1="no"
    else
      have_lib_$1="yes"
    fi

    AC_MSG_RESULT([$gb_cv_lib_$1])

    if test "$gb_cv_lib_$1" = "no"; then
      for gb_result in $gb_file_list; do
        GB_WARNING([Unable to find file: $gb_result])
      done
    fi

    $2_LIB=""
    $2_LDFLAGS=""
    $2_PATH=""

    for gb_dir in $gb_cv_lib_$1; do
      if test "x$$2_PATH" = "x"; then
        $2_PATH="$gb_dir/.."
      fi
      if test "$gb_dir" != "/lib"  && test "$gb_dir" != "/lib/"&& test "$gb_dir" != "/usr/lib" && test "$gb_dir" != "/usr/lib/"; then
        $2_LDFLAGS="$$2_LDFLAGS -L$gb_dir";
      fi
    done

    $2_LIB="$$2_LIB $7"

  fi

  if test "$have_inc_$1" = "yes" && test "$have_lib_$1" = "yes"; then

    have_$1=yes
    $2_DIR=$4
    AC_DEFINE(HAVE_$2_COMPONENT, 1, Have $3)

  else
  
    have_$1=no
    touch DISABLED
    touch DISABLED.$3
    
  fi
  
  if test "$have_$1" = "no"; then
  
    $2_INC=""
    $2_LIB=""
    $2_DIR=""
    $2_LDFLAGS=""
    if test x"$9" = x; then
      GB_WARNING([$3 is disabled])
    else
      GB_WARNING([$9])
    fi

  fi
  
  AC_SUBST($2_INC)
  AC_SUBST($2_LIB)
  AC_SUBST($2_LDFLAGS)
  AC_SUBST($2_DIR)
  AC_SUBST($2_PATH)
  
])


## ---------------------------------------------------------------------------
## GB_COMPONENT_SEARCH
## Component detection macro that uses GB_COMPONENT_PKG_CONFIG first, and
## then GB_COMPONENT.
##
##   $1  = Component key in lower case (ex: postgresql)
##   $2  = Component key in upper case (ex: POSTGRESQL)
##   $3  = Component name (ex: PostgreSQL)
##   $4  = Sub-directory name
##   $5  = pkg-config module name (optional)
##   $6  = How to get include path (must return it in gb_val)
##   $7  = How to get library path (must return it in gb_val)
##   $8  = Libraries
##   $9  = Compiler flags (optional)
##   $10 = Warning message (optional)
##
##   => defines HAVE_*_COMPONENT (to know if you can compile the component)
##      *_INC (for the compiler) and *_LIB (for the linker)
## ---------------------------------------------------------------------------

AC_DEFUN([GB_COMPONENT_SEARCH],
[
gb_in_component_search=yes
  GB_COMPONENT_PKG_CONFIG(
    $1,
    $2,
    $3,
    $4,
    $5,
    $10
  )
gb_in_component_search=no
  if test -z "${$2_LIB}"; then
    GB_COMPONENT(
      $1,
      $2,
      $3,
      $4,
      $6,
      $7,
      $8,
      $9,
      $10
    )
  fi
])


## ---------------------------------------------------------------------------
## GB_FIND_QT_MOC
## Find QT moc compiler
##
##   $1 = QT version
##   $2 = components to disable
##
##   Returns a path list in $gb_val
## ---------------------------------------------------------------------------

AC_DEFUN([GB_FIND_QT_MOC],
[
  gb_path_qt_moc=no
  if test x$1 = x; then
    gb_qt_version=3
  else
    gb_qt_version=$1
  fi
  
  AC_ARG_WITH(moc,
    [  --with-moc      The path to the QT moc compiler. ],
    [  gb_path_qt_moc="$withval" ])

  AC_MSG_CHECKING(for QT meta-object compiler)

  AC_CACHE_VAL(gb_cv_path_qt_moc, [

    gb_val=""
    if test "$gb_path_qt_moc" = no; then
      
      for gb_dir in $QTDIR /usr/lib/qt$gb_qt_version /usr/lib/qt/$gb_qt_version /usr/local/lib/qt$gb_qt_version /usr/local/lib/qt/$gb_qt_version /usr/local/qt$gb_qt_version /usr/local/qt/$gb_qt_version /usr/share/qt$gb_qt_version /usr/qt/$gb_qt_version /usr/pkg/qt$gb_qt_version /usr/pkg /usr; do
      
        gb_dir=$gb_dir/bin
      	
        if test -r "$gb_dir/moc"; then
          if test "x`$gb_dir/moc -v 2>&1 | grep " $gb_qt_version\."`" != x; then
            gb_val=$gb_dir/moc
            break
          fi
        fi
      
      done
      
      gb_path_qt_moc=$gb_val
    fi
    
    gb_cv_path_qt_moc=$gb_path_qt_moc
  ])

  AC_MSG_RESULT([$gb_cv_path_qt_moc])
  
  if test x"$gb_cv_path_qt_moc" = x; then
    GB_WARNING([QT moc compiler not found. Try --with-moc option.])
    MOC=""
    touch DISABLED 
  else
    MOC=$gb_cv_path_qt_moc
  fi
  
  AC_SUBST(MOC)
])

## ---------------------------------------------------------------------------
## GB_CHECK_XWINDOW
## Check the X-Window system installation
##
##   $1 = components to disable
## ---------------------------------------------------------------------------

AC_DEFUN([GB_CHECK_XWINDOW],
[
  AC_PATH_XTRA

  if test x"$have_x" = xyes; then
    if test -z `echo $X_LIBS | grep "\-lX11"`; then
      X_LIBS="$X_LIBS -lX11"
    fi
    if test -z `echo $X_LIBS | grep "\-lXext"`; then
      X_LIBS="$X_LIBS -lXext"
    fi
    X_LIBS="$X_PRE_LIBS $X_LIBS"
  else
    touch DISABLED
  fi

])

## ---------------------------------------------------------------------------
## Some macros
## ---------------------------------------------------------------------------

dnl Like AC_CHECK_HEADER, but it uses the already-computed -I directories.

AC_DEFUN([AC_CHECK_X_HEADER], [
  ac_save_CPPFLAGS="$CPPFLAGS"
  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_CHECK_HEADER([$1],[$2],[$3])
  CPPFLAGS="$ac_save_CPPFLAGS"
])

dnl Like AC_CHECK_LIB, but it used the -L dirs set up by the X checks.

AC_DEFUN([AC_CHECK_X_LIB], [
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LDFLAGS="$LDFLAGS"

  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi

  dnl note: $X_CFLAGS includes $x_includes
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"

  if test \! -z "$libdir" ; then
    LDFLAGS="$LDFLAGS -L$libdir"
  fi

  dnl note: $X_LIBS includes $x_libraries

  LDFLAGS="$LDFLAGS $X_LIBS"
  AC_CHECK_LIB([$1], [$2], [$3], [$4], [$5])
  CPPFLAGS="$ac_save_CPPFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"]
)

dnl Check if it is possible to turn off run time type information (RTTI)
AC_DEFUN([AC_PROG_CXX_FNO_RTTI],
[AC_CACHE_CHECK(whether ${CXX-g++} accepts -fno-rtti, ac_cv_prog_cxx_fno_rtti,
[echo 'void f(){}' > conftest.cc
if test -z "`${CXX-g++} -fno-rtti -c conftest.cc 2>&1`"; then
  ac_cv_prog_cxx_fno_rtti=yes
  CXXFLAGS="${CXXFLAGS} -fno-rtti"
else
  ac_cv_prog_cxx_fno_rtti=no
fi
rm -f conftest*
])])

dnl Check if the type socklen_t is defined anywhere
AC_DEFUN([AC_C_SOCKLEN_T],
[AC_CACHE_CHECK(for socklen_t, ac_cv_c_socklen_t,
[ AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
],[
socklen_t foo;
],[
  ac_cv_c_socklen_t=yes
],[
  ac_cv_c_socklen_t=no
  AC_DEFINE(socklen_t,int)
])])])

dnl Check for sys_errlist[] and sys_nerr, check for declaration
dnl Check nicked from aclocal.m4 from GNU bash 2.01
AC_DEFUN([AC_SYS_ERRLIST],
[AC_MSG_CHECKING([for sys_errlist and sys_nerr])
AC_CACHE_VAL(ac_cv_sys_errlist,
[AC_TRY_LINK([#include <errno.h>],
[extern char *sys_errlist[];
 extern int sys_nerr;
 char *msg = sys_errlist[sys_nerr - 1];],
    ac_cv_sys_errlist=yes, ac_cv_sys_errlist=no)])dnl
AC_MSG_RESULT($ac_cv_sys_errlist)
if test $ac_cv_sys_errlist = yes; then
AC_DEFINE(HAVE_SYS_ERRLIST)
fi
])

dnl @synopsis AX_CFLAGS_GCC_OPTION (optionflag [,[shellvar][,[A][,[NA]]])
dnl
dnl AX_CFLAGS_GCC_OPTION(-fvomit-frame) would show a message as like
dnl "checking CFLAGS for gcc -fvomit-frame ... yes" and adds the
dnl optionflag to CFLAGS if it is understood. You can override the
dnl shellvar-default of CFLAGS of course. The order of arguments stems
dnl from the explicit macros like AX_CFLAGS_WARN_ALL.
dnl
dnl The cousin AX_CXXFLAGS_GCC_OPTION would check for an option to add
dnl to CXXFLAGS - and it uses the autoconf setup for C++ instead of C
dnl (since it is possible to use different compilers for C and C++).
dnl
dnl The macro is a lot simpler than any special AX_CFLAGS_* macro (or
dnl ac_cxx_rtti.m4 macro) but allows to check for arbitrary options.
dnl However, if you use this macro in a few places, it would be great
dnl if you would make up a new function-macro and submit it to the
dnl ac-archive.
dnl
dnl   - $1 option-to-check-for : required ("-option" as non-value)
dnl   - $2 shell-variable-to-add-to : CFLAGS (or CXXFLAGS in the other case)
dnl   - $3 action-if-found : add value to shellvariable
dnl   - $4 action-if-not-found : nothing
dnl
dnl note: in earlier versions, $1-$2 were swapped. We try to detect the
dnl situation and accept a $2=~/-/ as being the old
dnl option-to-check-for.
dnl
dnl also: there are other variants that emerged from the original macro
dnl variant which did just test an option to be possibly added.
dnl However, some compilers accept an option silently, or possibly for
dnl just another option that was not intended. Therefore, we have to do
dnl a generic test for a compiler family. For gcc we check "-pedantic"
dnl being accepted which is also understood by compilers who just want
dnl to be compatible with gcc even when not being made from gcc
dnl sources.
dnl
dnl see also:
dnl
dnl       AX_CFLAGS_SUN_OPTION               AX_CFLAGS_HPUX_OPTION
dnl       AX_CFLAGS_AIX_OPTION               AX_CFLAGS_IRIX_OPTION
dnl
dnl @category C
dnl @author Guido Draheim <guidod@gmx.de>
dnl @version 2003-11-04
dnl @license GPLWithACException

AC_DEFUN([AX_CFLAGS_GCC_OPTION_OLD], [dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cflags_gcc_option_$2])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for gcc m4_ifval($2,$2,-option)],
VAR,[VAR="no, unknown"
 AC_LANG_SAVE
 AC_LANG_C
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic  % m4_ifval($2,$2,-option)"  dnl   GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])


dnl the only difference - the LANG selection... and the default FLAGS

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION_OLD], [dnl
AS_VAR_PUSHDEF([FLAGS],[CXXFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cxxflags_gcc_option_$2])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for gcc m4_ifval($2,$2,-option)],
VAR,[VAR="no, unknown"
 AC_LANG_SAVE
 AC_LANG_CXX
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic  % m4_ifval($2,$2,-option)"  dnl   GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])

dnl -------------------------------------------------------------------------

AC_DEFUN([AX_CFLAGS_GCC_OPTION_NEW], [dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cflags_gcc_option_$1])dnl
AC_CACHE_CHECK([m4_ifval($2,$2,FLAGS) for gcc m4_ifval($1,$1,-option)],
VAR,[VAR="no, unknown"
 AC_LANG_SAVE
 AC_LANG_C
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic  % m4_ifval($1,$1,-option)"  dnl   GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($2,$2,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($2,$2,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $VAR"])
                      m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])


dnl the only difference - the LANG selection... and the default FLAGS

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION_NEW], [dnl
AS_VAR_PUSHDEF([FLAGS],[CXXFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cxxflags_gcc_option_$1])dnl
AC_CACHE_CHECK([m4_ifval($2,$2,FLAGS) for gcc m4_ifval($1,$1,-option)],
VAR,[VAR="no, unknown"
 AC_LANG_SAVE
 AC_LANG_CXX
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic  % m4_ifval($1,$1,-option)"  dnl   GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($2,$2,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($2,$2,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $VAR"])
                      m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])

AC_DEFUN([AX_CFLAGS_GCC_OPTION],[ifelse(m4_bregexp([$2],[-]),-1,
[AX_CFLAGS_GCC_OPTION_NEW($@)],[AX_CFLAGS_GCC_OPTION_OLD($@)])])

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION],[ifelse(m4_bregexp([$2],[-]),-1,
[AX_CXXFLAGS_GCC_OPTION_NEW($@)],[AX_CXXFLAGS_GCC_OPTION_OLD($@)])])


