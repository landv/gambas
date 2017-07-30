gb_component(NATIVE gb.image.imlib
    REQUIRED_PACKAGES Imlib2
    SOURCE_LIST c_image.c  c_imlib.c  main.c)
gb_add_component_dependency(gb.image.imlib gb.draw)
