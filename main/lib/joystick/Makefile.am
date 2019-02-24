COMPONENT = gb.joystick
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.joystick.la
gb_joystick_la_LIBADD = @JOYSTICK_LIB@
gb_joystick_la_LDFLAGS = -module @LD_FLAGS@ @JOYSTICK_LDFLAGS@
gb_joystick_la_CPPFLAGS = @JOYSTICK_INC@ -I$(top_srcdir)/share

gb_joystick_la_SOURCES = \
  main.c main.h \
  c_joystick.c c_joystick.h
