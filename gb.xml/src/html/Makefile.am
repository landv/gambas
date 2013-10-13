COMPONENT = gb.xml.html
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.xml.html.la

##gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@ -l:gb.xml.so
##gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@ -L$(srcdir)/../.libs
gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@
gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@
gb_xml_html_la_CPPFLAGS = @XMLHTML_INC@
gb_xml_html_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_xml_html_la_SOURCES = *.h \
htmlmain.cpp \
htmldocument.cpp htmlelement.cpp \
CHTMLDocument.cpp CHTMLElement.cpp \
htmlserializer.cpp \
cssfilter.cpp \
htmlparser.cpp \
gb.xml.html.h
