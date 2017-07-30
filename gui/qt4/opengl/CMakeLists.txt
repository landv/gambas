set(MOC_HEADER_LIST CGLarea.h)

set(SRC_LIST CGLarea.cpp  main.cpp)

gb_add_qt4_component(gb.qt4.opengl QtGui QtOpenGL)
gb_component_register_switcher(gb.qt4.opengl gb.gui.opengl)
gb_add_component_dependency(gb.qt4.opengl gb.opengl)
