COMPONENT = gb.xml
include $(top_srcdir)/component.am
 
SUBDIRS = . @XMLHTML_DIR@  @XMLXSLT_DIR@ rpc
gblib_LTLIBRARIES = gb.xml.la
INCLUDES = @XML_INC@
 
gb_xml_la_LIBADD = @XML_LIB@ 
gb_xml_la_LDFLAGS = -module  @LD_FLAGS@ @XML_LDFLAGS@ 
gb_xml_la_CXXFLAGS = -Wall -fexceptions -pedantic -Wextra -Woverloaded-virtual\
-Wfloat-equal -Wno-write-strings -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion \
-Wshadow -Wredundant-decls -Wdouble-promotion -Winit-self -Wswitch-default \
-Wswitch-enum -Wundef -Wlogical-op -Winline -Wno-missing-field-initializers -Wno-unused-parameter -g -ggdb
 
gb_xml_la_SOURCES = main.cpp main.h gbi.cpp gbi.h\
document.cpp document.h node.cpp node.h textnode.cpp textnode.h element.cpp element.h\
CDocument.cpp CDocument.h CNode.cpp CNode.h CElement.h CElement.cpp CTextNode.h CTextNode.cpp\
CReader.h CReader.cpp reader.cpp reader.h\
CExplorer.h CExplorer.cpp explorer.cpp explorer.h
