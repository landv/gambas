
gb_add_component(gb.xml.xslt CXSLT.cpp main.cpp) 
gb_component_require_package(gb.xml.xslt LibXml2)
gb_component_require_package(gb.xml.xslt LibXslt)
target_include_directories(gb.xml.xslt PUBLIC "/usr/include/libxml2")