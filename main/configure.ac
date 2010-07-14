dnl ---- configure.ac for main programs

AC_INIT
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(main)
LT_INIT 
AM_PROG_CC_C_O

dnl ---- Check for internationalization library

GB_COMPONENT(
  intl,
  INTL,
  [external internationalization library],
  [],
  [GB_FIND(libintl.h, /opt/local /usr/local /usr, include)],
  [GB_FIND(libintl.$SHLIBEXT, /opt/local /usr/local /usr /, lib)],
  [-lintl],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])

dnl ---- Check for charset conversion library

GB_COMPONENT(
  conv,
  CONV,
  [optional external charset conversion library],
  [],
  [GB_FIND(iconv.h, /opt/local /usr/local /usr, include)],
  [GB_FIND(libiconv.$SHLIBEXT, /opt/local /usr/local /usr, lib)],
  [-liconv],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])

dnl ---- Check for gettext library

if test "x$GETTEXT_LIB" != x; then

GB_COMPONENT(
  gettext,
  GETTEXT,
  [external gettext library],
  [],
  [],
  [GB_FIND(libgettextlib.$SHLIBEXT, /opt/local /usr/local /usr, lib)],
  [-lgettextlib],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])
  
fi

dnl ---- Check for ffi library

GB_COMPONENT_SEARCH(
  ffi,
  FFI,
  [foreign function interface],
  [],
  libffi,
  [GB_FIND(ffi.h, /usr/local /usr/local/lib /usr /usr/lib /usr/lib/gcc/*/*, include ffi/include)],
  [GB_FIND(libffi.$SHLIBEXT, /usr/local /usr /usr/lib/gcc/*/*, lib .)],
  [-lffi])

if test -z "$FFI_LIB"; then
  GB_COMPONENT(
    ffi,
    FFI,
    [foreign function interface],
    [],
    [GB_FIND(ffi.h, /opt/local /usr/local /usr/local/lib /usr /usr/lib /usr/lib/gcc/*/*, include ffi/include)],
    [GB_FIND(libffi.$SHLIBEXT, /usr/local /usr, lib)],
    [-lffi])
fi

dnl ---- Remove DISABLED file that could have been generated before

rm -f DISABLED

dnl ---- We do not use libtool to load shared libraries anymore!

AC_DEFINE(DONT_USE_LTDL, 1, [Do not use libtool to load shared libraries])
if test "$SYSTEM" != "OPENBSD" && test "$SYSTEM" != "FREEBSD"; then
  DL_LIB="-ldl"
else
  DL_LIB=""
fi

AC_SUBST(DL_LIB)

dnl ---- Check for Portland scripts

AC_CHECK_PROGS(XDG_UTILS, [xdg-mime xdg-icon-resource], [])

dnl ---- Other options

AC_ARG_ENABLE(
  preloading,
  [  --enable-preloading            enable preloading (default: no)],
  gambas_preloading=$enableval,
  gambas_preloading=no
)

if test "$SYSTEM" != "CYGWIN"; then
  if test "$gambas_preloading" = "yes"; then
    AC_DEFINE(DO_PRELOADING, 1, allows shared library preloading )
  fi
else
  if test "$gambas_preloading" = "yes"; then
    GB_MESSAGE([Preloading is disabled on this system])
  fi
fi

dnl ---- Create makefiles

AC_CONFIG_FILES([\
Makefile \
share/Makefile \
gbc/Makefile \
gbx/Makefile \
lib/Makefile \
lib/debug/Makefile \
lib/eval/Makefile \
lib/db/Makefile \
lib/vb/Makefile \
lib/compress/Makefile \
lib/option/Makefile \
lib/draw/Makefile \
lib/gui/Makefile \
lib/image/Makefile \
lib/image.effect/Makefile \
lib/signal/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
