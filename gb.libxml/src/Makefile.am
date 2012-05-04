COMPONENT = gb.libxml
include $(top_srcdir)/component.am

EXTRA_DIST = *.kateproject

gblib_LTLIBRARIES = gb.libxml.la

gb_libxml_la_LIBADD = @XML_LIB@
gb_libxml_la_LDFLAGS = -module @LD_FLAGS@ @XML_LDFLAGS@
gb_libxml_la_CPPFLAGS = @XML_INC@

gb_libxml_la_SOURCES = \
 main.h main.c CXMLNode.h CXMLNode.c CXMLReader.h CXMLReader.c \
 CXMLWriter.h CXMLWriter.c CXMLDocument.h CXMLDocument.c 

