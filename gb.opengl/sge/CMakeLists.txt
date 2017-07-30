set(SRC_LIST cmd2model.c  cmd2object.c  main.c)

gb_component_require_package(gb.opengl.sge OpenGL)
gb_component_require_package(gb.opengl.sge GLEW)

gb_add_component(gb.opengl.sge ${SRC_LIST})
gb_add_component_dependency(gb.opengl.sge gb.image)