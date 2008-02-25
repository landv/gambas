COMPONENT = gb.xml
include $(top_srcdir)/component.am

SUBDIRS = @XSLT_DIR@ rpc

INCLUDES = @XML_INC@

EXTRA_DIST = *.kateproject

gblib_LTLIBRARIES = gb.xml.la

gb_xml_la_LIBADD = @XML_LIB@
gb_xml_la_LDFLAGS = -module @LD_FLAGS@ @XML_LDFLAGS@

gb_xml_la_SOURCES = main.h main.c CXMLNode.h CXMLNode.c CXMLReader.h CXMLReader.c \
                    CXMLWriter.h CXMLWriter.c CXMLDocument.h CXMLDocument.c 

