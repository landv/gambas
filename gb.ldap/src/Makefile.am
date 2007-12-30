COMPONENT = gb.ldap
include $(top_srcdir)/component.am

INCLUDES = @LDAP_INC@

gblib_LTLIBRARIES = gb.ldap.la

gb_ldap_la_LIBADD = @LDAP_LIB@
gb_ldap_la_LDFLAGS = -module @LD_FLAGS@

gb_ldap_la_SOURCES = main.c main.h Cldap.c Cldap.h
