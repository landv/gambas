macro(gb_prepare_qt4_component _COMPONENT)
    if(NOT DEFINED Qt4_FOUND)
        set(CMAKE_INCLUDE_CURRENT_DIR ON)

        include("FindQt4" OPTIONAL RESULT_VARIABLE FOUND_PACKAGE)
        
        if(${FOUND_PACKAGE} STREQUAL "NOTFOUND")
            gb_disable_component(${_COMPONENT})
            return()
        endif()
        
        find_package(Qt4 4.5 COMPONENTS ${ARGN})
    endif()
    
    if(NOT ${Qt4_FOUND})
        gb_disable_component(${_COMPONENT})
        return()
    endif()
    
    foreach(l ${ARGN})
        string(TOUPPER ${l} lu)
        if(NOT "${QT_${lu}_FOUND}")
            gb_disable_component(${_COMPONENT})
            return()
        endif()
    endforeach()
    
    #gb_component_require_package(${_COMPONENT} X11)
    
    SET(PREFIXED_ARGN)
    
    # prefix modules list
    foreach(l ${ARGN})
      list(APPEND PREFIXED_ARGN Qt4::${l})
    endforeach()
    
    set(MOC_FILES "")
    qt4_wrap_cpp(MOC_FILES ${MOC_HEADER_LIST})
endmacro()

macro(gb_add_qt4_impl _COMPONENT)
    gb_prepare_qt4_component(${_COMPONENT} ${ARGN})
    
    gb_add_gui_impl(${_COMPONENT} ${SRC_LIST} ${MOC_FILES})

    target_link_libraries(${_COMPONENT} ${PREFIXED_ARGN})
endmacro()

macro(gb_add_qt4_component _COMPONENT)
    gb_prepare_qt4_component(${_COMPONENT} ${ARGN})
    
    gb_add_gui_component(${_COMPONENT} ${SRC_LIST} ${MOC_FILES})

    target_link_libraries(${_COMPONENT} ${PREFIXED_ARGN})
endmacro()

set(MOC_HEADER_LIST main.h
    CWidget.h CWindow.h CButton.h CContainer.h CLabel.h 
    CTextBox.h CPictureBox.h CMenu.h
    CCheckBox.h CFrame.h CRadioButton.h
    CTextArea.h CTabStrip.h
    CClipboard.h CWatch.h CScrollView.h CDrawingArea.h
    CMessage.h CSlider.h CScrollBar.h
    CSpinBox.h ctrayicon.h CWatcher.h CEmbedder.h)
    
set(SRC_LIST 
CButton.cpp       CFrame.cpp       CPicture.cpp      CTextArea.cpp
CCheckBox.cpp     CImage.cpp       cprinter.cpp      CTextBox.cpp
CClipboard.cpp    CKey.cpp         CRadioButton.cpp  ctrayicon.cpp
CColor.cpp        CLabel.cpp       CScreen.cpp       CWatch.cpp
CConst.cpp        CMenu.cpp        CScrollBar.cpp    CWatcher.cpp
CContainer.cpp    CMessage.cpp     CScrollView.cpp   CWidget.cpp
CDialog.cpp       CMouse.cpp       CSlider.cpp       CWindow.cpp
CDraw.cpp         CMovieBox.cpp    CSpinBox.cpp      desktop.c
CDrawingArea.cpp  cpaint_impl.cpp  CStyle.cpp        fix_breeze.cpp
CEmbedder.cpp     CPanel.cpp       csvgimage.cpp     main.cpp
CFont.cpp         CPictureBox.cpp  CTabStrip.cpp     x11.c)

gb_add_qt4_impl(gb.qt4 QtGui QtSvg)
gb_component_register_switcher(gb.qt4 gb.gui.qt)
gb_add_component_dependency(gb.qt4 gb.image)
gb_add_component_dependency(gb.qt4 gb.draw)
gb_add_component_dependency(gb.qt4 gb.geom)
gb_add_subcomponent(gb.qt4 ext)
gb_add_subcomponent(gb.qt4 webkit)
gb_add_subcomponent(gb.qt4 opengl)
# gb_add_component_requirement(gb.qt4 gb.gui.base)
