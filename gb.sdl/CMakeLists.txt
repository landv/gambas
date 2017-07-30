gb_component(NATIVE gb.sdl
    COMPONENT_DEPENDENCIES gb.image
    REQUIRED_PACKAGES SDL SDL_image SDL_ttf GLEW Xcursor
    SOURCE_LIST Cconst.cpp  Cdesktop.cpp  Cdraw.cpp  Cfont.cpp  Cimage.cpp
    Cjoystick.cpp  Ckey.cpp  Cmouse.cpp  Cwindow.cpp  main.cpp  SDLapp.cpp
    SDLcore.cpp  SDLcursor.cpp  SDLdebug.cpp  SDLerror.cpp  SDLfont.cpp
    SDLgfx.cpp  SDLosrender.cpp  SDLsurface.cpp  SDLtexture.cpp  SDLwindow.cpp)
