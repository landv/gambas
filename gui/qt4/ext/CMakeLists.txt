set(MOC_HEADER_LIST CDial.h 
CEditor.h  CTextEdit.h gview.h)
    
set(SRC_LIST CDial.cpp    CLCDNumber.cpp  garray.cpp     gstring.cpp  main.cpp
CEditor.cpp  CTextEdit.cpp   gdocument.cpp  gview.cpp)

gb_add_qt4_component(gb.qt4.ext QtGui Qt3Support)
gb_add_component_dependency(gb.qt4.ext gb.image)
gb_add_component_dependency(gb.qt4.ext gb.draw)
gb_add_component_dependency(gb.qt4.ext gb.eval)
gb_add_component_dependency(gb.qt4.ext gb.geom)
