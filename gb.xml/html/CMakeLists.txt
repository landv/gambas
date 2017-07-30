 
set(GB_XML_HTML_SRC_LIST
htmlmain.cpp 
htmldocument.cpp htmlelement.cpp 
CHTMLDocument.cpp CHTMLElement.cpp 
htmlserializer.cpp 
cssfilter.cpp 
htmlparser.cpp 
gb.xml.html.h)

gb_add_component(gb.xml.html ${GB_XML_HTML_SRC_LIST})