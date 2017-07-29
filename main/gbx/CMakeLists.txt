set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread -D_REENTRANT -Wl,--no-as-needed -DGAMBAS_PATH=\"\\\"${CMAKE_INSTALL_FULL_BINDIR}\\\"\"")

# add_definitions("")
find_library(M_LIB m REQUIRED)
find_package(LibFFI REQUIRED)

set(GBX_LIBS ${LIBFFI_LIBRARY} ${M_LIB} ${CMAKE_DL_LIBS} libgbx)
set(GBX_INCLUDES ${LIBFFI_INCLUDE_DIR}
    ${GAMBAS_INCLUDE_DIR} ${GAMBAS_SOURCE_CONFIG_DIR} 
    ${CMAKE_SOURCE_DIR}/main/share/ ${CMAKE_SOURCE_DIR}/main/gbx/ ${CMAKE_SOURCE_DIR}/main/lib/)

# libgbx

set (LIBGBX_SRC_LIST
 gb_error.h gb_error.c 
 gbx_split.h gbx_split.c
 gbx_exec_loop.c)
 
add_library(libgbx ${LIBGBX_SRC_LIST})
target_include_directories(libgbx PUBLIC ${GBX_INCLUDES})
set_target_properties(libgbx PROPERTIES PREFIX "")
    
# gbx
set(GBX_EXECUTABLE_NAME "gbx${GAMBAS_VERSION_MAJOR}")

set (GBX_SRC_LIST gb_common_check.h gb_common.c  
 gbx_debug.h gbx_debug.c  
 gbx_jit.h gbx_jit.c  
 gb_alloc.c gb_array.c  
 gbx_stack.h gbx_stack.c  
 gb_buffer.c gbx_replace.c  
 gbx_value.h gbx_value.c  
 gb_hash.c  
 gb_table.c  
 gb_list.c  
 gbx_type.h gbx_type.c  
 gbx_exec.h gbx_exec.c gbx_exec_push.c gbx_exec_enum.c gbx_exec_pop.c gbx_exec_operator.c  
 gbx_class_desc.h gbx_class.h gbx_class_init.c gbx_class.c gbx_class_native.c  
 gbx_class_load.c gbx_class_load.h  
 gbx_event.h gbx_event.c  
 gb_file.h gb_file.c  
 gbx_stream.h gbx_stream.c gbx_stream_direct.c gbx_stream_buffer.c gbx_stream_memory.c  
 gbx_stream_arch.c gbx_stream_process.c gbx_stream_pipe.c gbx_stream_string.c  
 gbx_project.h gbx_project.c  
 gbx_library.h gbx_library.c  
 gbx_subr.h gbx_subr.c  
 gbx_subr_file.c gbx_subr_string.c gbx_subr_conv.c gbx_subr_time.c gbx_subr_extern.c gbx_subr_misc.c  
 gbx_math.h gbx_math.c  
 gbx_subr_math_temp.h gbx_subr_math.c  
 gbx_subr_test_temp.h gbx_subr_test.c  
 gbx_api.h gbx_api.c  
 gbx_local.h gbx_local.c  
 gbx_regexp.h gbx_regexp.c  
 gbx_archive.h gbx_archive.c  
 gbx_watch.h gbx_watch.c  
 gbx_expression.h gbx_eval.h gbx_eval.c  
 gbx_compare.h gbx_compare.c  
 gbx.c  
 gbx_number.h gbx_number.c  
 gbx_object.h gbx_object.c  
 gbx_string.h gbx_string.c  
 gbx_variant.h  
 gbx_date.h gbx_date.c  
 gbx_c_class.h gbx_c_class.c  
 gbx_c_collection.h gbx_c_collection.c  
 gbx_c_error.h gbx_c_error.c  
 gbx_c_gambas.h gbx_c_gambas.c  
 gbx_c_file.h gbx_c_file.c  
 gbx_c_application.h gbx_c_application.c  
 gbx_c_system.h gbx_c_system.c  
 gbx_c_array.h gbx_c_array.c  
 gbx_c_process.h gbx_c_process.c  
 gbx_c_string.h gbx_c_string.c  
 gbx_c_observer.h gbx_c_observer.c  
 gbx_c_task.h gbx_c_task.c  
 gbx_component.h gbx_component.c  
 gbx_extern.h gbx_extern.c  
 gbx_c_enum.h gbx_c_enum.c  
 gbx_c_timer.h gbx_c_timer.c  
 gbx_struct.h gbx_struct.c  
 gbx_signal.h gbx_signal.c)

add_executable(${GBX_EXECUTABLE_NAME} ${GBX_SRC_LIST})

target_link_libraries(${GBX_EXECUTABLE_NAME} ${GBX_LIBS})
target_include_directories(${GBX_EXECUTABLE_NAME} PUBLIC ${GBX_INCLUDES})

set(GB_SRC_LIST gbx_info.h 
 gbx_local.h gbx_compare.h gbx_date.h 
 gbx_c_class.h gbx_c_class.c 
 gbx_c_collection.h gbx_c_collection.c 
 gbx_c_error.h gbx_c_error.c 
 gbx_c_gambas.h gbx_c_gambas.c 
 gbx_c_file.h gbx_c_file.c 
 gbx_c_application.h gbx_c_application.c 
 gbx_c_system.h gbx_c_system.c 
 gbx_c_array.h gbx_c_array.c 
 gbx_c_process.h gbx_c_process.c 
 gbx_c_string.h gbx_c_string.c 
 gbx_c_observer.h gbx_c_observer.c 
 gbx_c_enum.h gbx_c_enum.c 
 gbx_c_timer.h gbx_c_timer.c 
 gbx_c_task.h gbx_c_task.c 
 gbx_class_info.c)
 
 
gb_add_component(gb ${GB_SRC_LIST})
target_include_directories(gb PUBLIC ${GBX_INCLUDES})
target_compile_definitions(gb PUBLIC -DGBX_INFO)

install(TARGETS ${GBX_EXECUTABLE_NAME}
        RUNTIME DESTINATION bin)
# add_definitions("-Wall")
