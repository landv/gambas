
macro(gb_add_lib_component _NAME)
    gb_add_component(${_NAME} ${ARGN})
    target_include_directories(${_NAME} PUBLIC 
        ${GAMBAS_SOURCE_ROOT}/main/share/
        ${GAMBAS_SOURCE_ROOT}/main/gbx/
        ${GAMBAS_SOURCE_ROOT}/main/lib/)
endmacro()

# add_subdirectory(jit) 
# add_subdirectory(eval) 
# add_subdirectory(debug) 
# add_subdirectory(data) 
