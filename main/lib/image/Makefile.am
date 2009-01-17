COMPONENT = gb.image
include $(top_srcdir)/component.am

INCLUDES = -I$(top_srcdir)/share @INCLTDL@

gblib_LTLIBRARIES = gb.image.la

gb_image_la_LIBADD = 
gb_image_la_LDFLAGS = -module @LD_FLAGS@

gb_image_la_SOURCES = \
 image_stat.h image_stat.c \
 image.h image.c \
 CImage.h CImage.c \
 CImageStat.h CImageStat.c \
 gb.image.h \
 main.h main.c

image.lo: image.c
	$(LTCOMPILE) -O3 $(CFLAGS) -MT image.o -MD -MP -MF $(DEPDIR)/image.Tpo -c -o image.o image.c
	mv -f $(DEPDIR)/image.Tpo $(DEPDIR)/image.Po


