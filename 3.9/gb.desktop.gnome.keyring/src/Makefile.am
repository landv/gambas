COMPONENT = gb.desktop.gnome.keyring
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.desktop.gnome.keyring.la

gb_desktop_gnome_keyring_la_LIBADD = @DESKTOP_GNOME_KEYRING_LIB@
gb_desktop_gnome_keyring_la_LDFLAGS = -module @LD_FLAGS@ @DESKTOP_GNOME_KEYRING_LDFLAGS@
gb_desktop_gnome_keyring_la_CPPFLAGS = @DESKTOP_GNOME_KEYRING_INC@ 

gb_desktop_gnome_keyring_la_SOURCES = main.h main.c keyring.c keyring.h

