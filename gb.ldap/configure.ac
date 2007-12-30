dnl ---- configure.ac for gb.ldap

AC_INIT(configure.ac)
GB_INIT(gb.ldap)
AC_PROG_LIBTOOL
GB_COMPONENT(
  ldap,
  LDAP,
  [Ldap component],
  [src],
  [GB_FIND(ldap.h, /usr/local/lib /usr/local /usr/lib /usr, ldap/include include ldap*/include ldap)],
  [GB_FIND(libldap.$SHLIBEXT, /usr/local /usr, lib)],
  [$C_LIB -lldap])



AC_OUTPUT( Makefile src/Makefile )
