COMPONENT = gb.xml.xslt
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.xml.xslt.la

gb_xml_xslt_la_LIBADD = @XMLXSLT_LIB@ @XML_LIB@
gb_xml_xslt_la_LDFLAGS = -module @LD_FLAGS@ @XML_LDFLAGS@ @XMLXSLT_LDFLAGS@
gb_xml_xslt_la_CPPFLAGS = @XMLXSLT_INC@ 
gb_xml_xslt_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_xml_xslt_la_SOURCES = main.cpp CXSLT.h CXSLT.cpp
