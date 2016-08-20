COMPONENT = gb.dbus
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.dbus.la

gb_dbus_la_LIBADD = @DBUS_LIB@
gb_dbus_la_LDFLAGS = -module @LD_FLAGS@ @DBUS_LDFLAGS@
gb_dbus_la_CPPFLAGS = @DBUS_INC@

gb_dbus_la_SOURCES = \
	main.c main.h \
	helper.c helper.h \
	dbus_print_message.c dbus_print_message.h \
	c_dbus.c c_dbus.h \
	c_dbusvariant.c c_dbusvariant.h \
	c_dbusconnection.c c_dbusconnection.h \
	c_dbusobserver.c c_dbusobserver.h
