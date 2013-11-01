COMPONENT = gb.desktop.gnome
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.desktop.gnome.la

gb_desktop_gnome_la_LIBADD = @GNOME_LIB@
gb_desktop_gnome_la_LDFLAGS = -module @LD_FLAGS@ @GNOME_LDFLAGS@
gb_desktop_gnome_la_CPPFLAGS = @GNOME_INC@ 

gb_desktop_gnome_la_SOURCES = main.h main.c keyring.c keyring.h

