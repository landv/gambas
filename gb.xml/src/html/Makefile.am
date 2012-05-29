COMPONENT = gb.xml.html
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.xml.html.la

##gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@ -l:gb.xml.so
##gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@ -L$(srcdir)/../.libs
gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@
gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@
gb_xml_html_la_CPPFLAGS = @XMLHTML_INC@
gb_xml_html_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_xml_la_SOURCES = ../gbi.cpp ../gbi.h ../utils.cpp ../utils.h\
../document.cpp ../document.h ../node.cpp ../node.h ../textnode.cpp ../textnode.h ../element.cpp ../element.h

gb_xml_html_la_SOURCES = $(gb_xml_la_SOURCES) main.cpp main.h \
hdocument.cpp hdocument.h helement.cpp helement.h \
CDocument.cpp CDocument.h CElement.h CElement.cpp
