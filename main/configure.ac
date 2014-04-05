dnl ---- configure.ac for main programs

m4_include([../version.m4])
AC_INIT(gambas3-main, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(main)
GB_MATH_FUNC
LT_INIT 
AM_PROG_CC_C_O

dnl ---- Check for internationalization library

GB_COMPONENT(
  intl, INTL, libintl, [],
  [GB_FIND(libintl.h, /opt/local /usr/local /usr, include)],
  [GB_FIND(libintl.$SHLIBEXT, /opt/local /usr/local /usr /, lib)],
  [-lintl],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])

dnl ---- Remove DISABLED file that could have been generated before

rm -f DISABLED DISABLED.*

dnl ---- Check for charset conversion library

GB_COMPONENT(
  conv, CONV, libiconv, [],
  [GB_FIND(iconv.h, /opt/local /usr/local /usr, include)],
  [GB_FIND(libiconv.$SHLIBEXT, /opt/local /usr/local /usr, lib)],
  [-liconv],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])

dnl ---- Remove DISABLED file that could have been generated before

rm -f DISABLED DISABLED.*

dnl ---- Check for gettext library

if test "x$GETTEXT_LIB" != x; then

GB_COMPONENT(
  gettext, GETTEXT, libgettextlib, [],
  [],
  [GB_FIND(libgettextlib.$SHLIBEXT, /opt/local /usr/local /usr, lib)],
  [-lgettextlib],
  [],
  [This library may be located inside the system C library, so you can ignore that warning...])
  
fi

dnl ---- Remove DISABLED file that could have been generated before

rm -f DISABLED DISABLED.*

dnl ---- Check for ffi library

GB_COMPONENT_SEARCH(
  ffi, FFI, libffi, [],
  libffi,
  [GB_FIND(ffi.h, /usr/local /usr/local/lib /usr /usr/lib /usr/lib/gcc/*/*, include ffi/include)],
  [GB_FIND(libffi.$SHLIBEXT, /usr/local /usr /usr/lib/gcc/*/*, lib .)],
  [-lffi])

if test -z "$FFI_LIB"; then
  GB_COMPONENT(
    ffi, FFI, libffi, [],
    [GB_FIND(ffi.h, /opt/local /usr/local /usr/local/lib /usr /usr/lib /usr/lib/gcc/*/*, include ffi/include)],
    [GB_FIND(libffi.$SHLIBEXT, /usr/local /usr, lib)],
    [-lffi])
fi

dnl ---- Remove DISABLED file that could have been generated before

rm -f DISABLED DISABLED.*

dnl ---- Check for the Linux inotify headers

GB_COMPONENT(
  inotify, INOTIFY, gb.inotify, [inotify],
  [GB_FIND(sys/inotify.h, /usr/local/lib /usr/local /usr/lib /usr, include)],
  [GB_FIND(, /usr/local /usr, lib)],
  [$C_LIB])

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
lib/geom/Makefile \
lib/draw/Makefile \
lib/gui/Makefile \
lib/gui.opengl/Makefile \
lib/image/Makefile \
lib/image.effect/Makefile \
lib/signal/Makefile \
lib/complex/Makefile \
lib/data/Makefile \
lib/clipper/Makefile \
lib/inotify/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
