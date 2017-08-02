gb_component(NATIVE gb.debug
    SOURCE_LIST CDebug.c print.c debug.c profile.c main.c)

target_include_directories(gb.debug PUBLIC 
    ${GAMBAS_SOURCE_ROOT}/main/share/
    ${GAMBAS_SOURCE_ROOT}/main/gbx/
    ${GAMBAS_SOURCE_ROOT}/main/lib/)
