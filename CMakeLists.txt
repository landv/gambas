project(Gambas)
cmake_minimum_required(VERSION 3.3)

set(GAMBAS_SOURCE_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_INSTALL_PREFIX "/usr/" CACHE INTERNAL "" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb -fvisibility=hidden")

include(config/gambas.cmake)

add_subdirectory(main)
add_subdirectory(gb.xml)
# add_subdirectory(comp)
# add_subdirectory(app)

gb_check_needs()
gb_check_dependencies()

gb_disable_component(gb.gui.base)

get_property(GB_DISABLED_COMPONENTS GLOBAL PROPERTY GB_DISABLED_COMPONENTS)

if(GB_DISABLED_COMPONENTS)
    message("||==== THE FOLLOWING COMPONENTS ARE DISABLED : ====||")
    foreach(disabled_component IN ITEMS ${GB_DISABLED_COMPONENTS})
        
        message("|| >>  " ${disabled_component})
        
    endforeach()
endif()
