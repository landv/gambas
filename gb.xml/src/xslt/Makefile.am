COMPONENT = gb.xml.xslt
include $(top_srcdir)/component.am

INCLUDES = @XMLXSLT_INC@ 


gblib_LTLIBRARIES = gb.xml.xslt.la

gb_xml_xslt_la_LIBADD = @XMLXSLT_LIB@ @XML_LIB@
gb_xml_xslt_la_LDFLAGS = -module @LD_FLAGS@ @XML_LDFLAGS@ @XMLXSLT_LDFLAGS@
gb_xml_xslt_la_CXXFLAGS = -fexceptions -pedantic -g -ggdb

gb_xml_xslt_la_SOURCES = main.h main.cpp CXSLT.h CXSLT.cpp

