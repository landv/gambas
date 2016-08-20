COMPONENT = gb.desktop.x11
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.desktop.x11.la

gb_desktop_x11_la_LIBADD = @DESKTOP_X11_LIB@
gb_desktop_x11_la_LDFLAGS = -module @LD_FLAGS@ @DESKTOP_X11_LDFLAGS@
gb_desktop_x11_la_CPPFLAGS = @DESKTOP_X11_INC@

gb_desktop_x11_la_SOURCES = \
 main.c main.h \
 x11.c x11.h \
 gb_list.c \
 c_x11.c c_x11.h \
 c_x11systray.h c_x11systray.c \
 systray/systray.h \
 systray/systray.c \
 systray/icons.c \
 systray/icons.h \
 systray/xembed.c \
 systray/xembed.h \
 systray/embed.c \
 systray/embed.h \
 systray/settings.c \
 systray/settings.h \
 systray/xutils.c \
 systray/xutils.h \
 systray/tray.c \
 systray/tray.h \
 systray/debug.h \
 systray/list.h \
 systray/wmh.c \
 systray/wmh.h \
 systray/common.h \
 systray/debug.c \
 systray/kde_tray.c \
 systray/kde_tray.h
 