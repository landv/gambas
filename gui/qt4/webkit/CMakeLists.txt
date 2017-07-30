set(MOC_HEADER_LIST ccookiejar.h  cwebdownload.h    cwebview.h )
    
set(SRC_LIST ccookiejar.cpp  cwebdownload.cpp  cwebelement.cpp  cwebframe.cpp  cwebhittest.cpp  cwebsettings.cpp  cwebview.cpp  main.cpp)

gb_add_qt4_component(gb.qt4.webkit QtGui QtWebKit)
gb_component_register_switcher(gb.qt4.webkit gb.gui.qt.webkit)
