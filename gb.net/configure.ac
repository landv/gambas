dnl ---- configure.ac for gb.net

AC_INIT(configure.ac)
GB_INIT(gb.net)
AC_PROG_LIBTOOL

GB_COMPONENT(
  net,
  NET,
  [Networking component],
  [src],
  [],
  [],
  [$C_LIB $THREAD_LIB],
  [$THREAD_INC])

AC_OUTPUT(Makefile src/Makefile)

dnl GB_COMPONENT_PKG_CONFIG(
dnl   smtp,
dnl   SMTP,
dnl   [STMP client],
dnl   [src],
dnl   glib-2.0)

dnl GB_COMPONENT(
dnl   pop3,
dnl   POP3,
dnl   [POP3 client component],
dnl   [pop3],
dnl   [GB_FIND(libspopc.h, /usr /usr/local, include)],
dnl   [GB_FIND(libspopc.$SHLIBEXT, /usr /usr/local, lib)],
dnl   [-lspopc])

dnl AC_OUTPUT( \
dnl Makefile \
dnl src/Makefile \
dnl src/smtp/Makefile \
dnl src/pop3/Makefile \
dnl )
