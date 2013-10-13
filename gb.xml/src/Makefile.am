COMPONENT = gb.xml
include $(top_srcdir)/component.am
 
SUBDIRS = .   @XMLXSLT_DIR@ rpc @XMLHTML_DIR@
gblib_LTLIBRARIES = gb.xml.la
 
gb_xml_la_LIBADD = @XML_LIB@ 
gb_xml_la_LDFLAGS = -module  @LD_FLAGS@ @XML_LDFLAGS@ 
gb_xml_la_CPPFLAGS = @XML_INC@
gb_xml_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions -fvisibility=default

gb_xml_la_SOURCES = main.cpp main.h utils.cpp utils.h\
serializer.h serializer.cpp parser.h parser.cpp\
document.cpp document.h node.cpp node.h textnode.cpp textnode.h element.cpp element.h \
CDocument.cpp CDocument.h CNode.cpp CNode.h CElement.h CElement.cpp CTextNode.h CTextNode.cpp \
CReader.h CReader.cpp reader.cpp reader.h \
CExplorer.h CExplorer.cpp explorer.cpp explorer.h \
gb.xml.h gbinterface.h
