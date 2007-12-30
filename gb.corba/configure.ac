dnl ---- configure.ac for gb.corba

AC_INIT(configure.ac)
GB_INIT(gb.corba)
AC_PROG_LIBTOOL
GB_COMPONENT(
  corba,
  CORBA,
  [CORBA component],
  [src],
  [GB_FIND(CORBA.h, /usr/share/ace /usr/local/lib /usr/local /usr/lib /usr, omniORB4/include include/omniORB4 include)],
  [GB_FIND(libomniORB4.$SHLIBEXT libomniDynamic4.$SHLIBEXT, /usr/share/ace /usr/local /usr, lib)],
  [$THREAD_LIB -D_MT -DHAVE_CONFIG_FILE -lomniORB4 -lomniDynamic4 ])



AC_OUTPUT( Makefile src/Makefile )
