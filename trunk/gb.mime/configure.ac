dnl ---- configure.ac for gb.mime

m4_include([../version.m4])
AC_INIT(gambas3-gb-mime, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.mime)
AC_PROG_LIBTOOL

gb_in_component_search=yes

GB_COMPONENT_PKG_CONFIG(
  mime,
  MIME,
  gb.mime,
  [src],
  gmime-2.6)

gb_in_component_search=no

if test "$have_mime" = "no"; then

  rm -f DISABLED DISABLED.gb.mime

  GB_COMPONENT_PKG_CONFIG(
    mime,
    MIME,
    gb.mime,
    [src],
    gmime-2.4)

  if test "$have_mime" = "yes"; then
    GB_WARNING([But gmime 2.4 has been detected!])
  fi

fi

AC_OUTPUT(Makefile src/Makefile)

GB_PRINT_MESSAGES
