gb_component(NATIVE gb.media
    COMPONENT_DEPENDENCIES gb.image
    REQUIRED_PACKAGES "GStreamer COMPONENTS gstreamer-video"
    SOURCE_LIST c_media.c  c_mediaplayer.c  c_mediavideo.c  main.c)
