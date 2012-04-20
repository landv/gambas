COMPONENT = gb.xml.html
include $(top_srcdir)/component.am
gblib_LTLIBRARIES = gb.xml.html.la
INCLUDES = @XMLHTML_INC@

gb_xml_html_la_LIBADD = @XMLHTML_LIB@ @XML_LIB@
gb_xml_html_la_LDFLAGS = -module  @LD_FLAGS@ @XMLHTML_LDFLAGS@ 
gb_xml_html_la_CXXFLAGS = -Wall -fexceptions -pedantic -Wextra -Woverloaded-virtual\
-Wfloat-equal -Wno-write-strings -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion \
-Wshadow -Wredundant-decls -Wdouble-promotion -Winit-self -Wswitch-default \
-Wswitch-enum -Wundef -Wlogical-op -Winline -Wno-missing-field-initializers -Wno-unused-parameter -g -ggdb
 
gb_xml_html_la_SOURCES = main.cpp main.h\
document.cpp document.h element.cpp element.h\
CDocument.cpp CDocument.h CElement.h CElement.cpp
