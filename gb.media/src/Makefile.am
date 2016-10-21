COMPONENT = gb.media
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.media.la

gb_media_la_LIBADD = @THREAD_LIB@ @MEDIA_LIB@
gb_media_la_LDFLAGS = -module @LD_FLAGS@ @MEDIA_LDFLAGS@
gb_media_la_CPPFLAGS = @THREAD_INC@ @MEDIA_INC@

gb_media_la_SOURCES = \
 main.c main.h \
 c_media.c c_media.h \
 c_mediaplayer.c c_mediaplayer.h
