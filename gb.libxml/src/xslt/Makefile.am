COMPONENT = gb.libxml.xslt
include $(top_srcdir)/component.am

INCLUDES = @XSLT_INC@ 

EXTRA_DIST = *.kateproject

gblib_LTLIBRARIES = gb.libxml.xslt.la

gb_xml_xslt_la_LIBADD = @XSLT_LIB@
gb_xml_xslt_la_LDFLAGS = -module @LD_FLAGS@ @XSLT_LDFLAGS@

gb_xml_xslt_la_SOURCES = main.h main.c CXSLT.h CXSLT.c 

