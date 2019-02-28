COMPONENT = gb.input
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.input.la
gb_input_la_LIBADD = @INPUT_LIB@
gb_input_la_LDFLAGS = -module @LD_FLAGS@ @INPUT_LDFLAGS@
gb_input_la_CPPFLAGS = @INPUT_INC@
gb_input_la_SOURCES = main.c main.h evdev.c evdev.h
