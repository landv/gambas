COMPONENT = gb.xml.html
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.xml.html.la

##gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@ -l:gb.xml.so
##gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@ -L$(srcdir)/../.libs
gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@
gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@
gb_xml_html_la_CPPFLAGS = @XMLHTML_INC@
gb_xml_html_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_xml_html_la_SOURCES = htmlmain.cpp htmlmain.h \
htmldocument.cpp htmlelement.cpp \
CHTMLDocument.cpp CHTMLDocument.h CHTMLElement.h CHTMLElement.cpp \
htmlserializer.cpp htmlserializer.h \
cssfilter.cpp cssfilter.h \
htmlparser.cpp htmlparser.h \
gb.xml.html.h
