COMPONENT = gb.xml.html
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.xml.html.la

gb_xml_html_la_LIBADD = ../gb.xml.la @XMLHTML_LIB@ @XML_LIB@
gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@ 
gb_xml_html_la_CPPFLAGS = @XMLHTML_INC@
gb_xml_html_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_xml_html_la_SOURCES = main.cpp main.h \
document.cpp document.h element.cpp element.h \
CDocument.cpp CDocument.h CElement.h CElement.cpp
