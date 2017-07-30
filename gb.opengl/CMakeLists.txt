gb_component(NATIVE gb.opengl
    REQUIRED_PACKAGES OpenGL GLEW
    SUBCOMPONENTS glsl glu sge
    SOURCE_LIST framebufferobject.c  GL.c  GLclipping.c  GLcolorLighting.c
    GLcoordTransf.c  GLdisplayList.c  GLeval.c  GLfog.c  GLframeBufferOps.c
    GLinfo.c  GLmodesExec.c  GLpixelOperations.c  GLprimitives.c
    GLrasterization.c  GLselectFeedback.c  GLtextureMapping.c  main.c)
