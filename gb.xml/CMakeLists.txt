 
set(GB_XML_SRC_LIST
    main.cpp main.h utils.cpp utils.h
    serializer.h serializer.cpp parser.h parser.cpp
    document.cpp document.h node.cpp node.h textnode.cpp textnode.h element.cpp element.h 
    CDocument.cpp CDocument.h CNode.cpp CNode.h CElement.h CElement.cpp CTextNode.h CTextNode.cpp 
    CReader.h CReader.cpp reader.cpp reader.h 
    CExplorer.h CExplorer.cpp explorer.cpp explorer.h 
    gb.xml.h gbinterface.h
)

gb_component(NATIVE gb.xml GAMBAS gb.xml
    SOURCE_LIST ${GB_XML_SRC_LIST}
    SUBCOMPONENTS html xslt gb.xml.rpc)

