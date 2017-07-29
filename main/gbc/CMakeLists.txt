set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGAMBAS_PATH=\"\\\"${CMAKE_INSTALL_FULL_BINDIR}\\\"\"")

# gbc
set(GBC_EXECUTABLE_NAME "gbc${GAMBAS_VERSION_MAJOR}")

set(GBC_SRC_LIST 
 gb_error.h gb_error.c 
 gb_alloc.c gb_array.c 
 gbc_class.h gbc_class.c 
 gbc_read.h gbc_read.c 
 gbc_preprocess.h gbc_preprocess.c 
 gbc_type.h gbc_type.c 
 gbc_compile.h gbc_compile.c 
 gbc_header.h gbc_header.c 
 gbc_help.h gbc_help.c 
 gbc_output.h gbc_output.c 
 gbc_trans.h gbc_trans_expr.c gbc_trans_tree.c gbc_trans_ctrl.c gbc_trans_subr.c 
 gbc_reserved.c 
 gb_buffer.c 
 gbc_dump.c gbc_code.c gbc_trans.c gbc_trans_code.c 
 gbc_pcode.c 
 gb_file.h gb_file.c 
 gbc_form.h gbc_form.c gbc_form_webpage.c 
 gb_str.h gb_str.c 
 gbc_chown.h gbc_chown.c 
 gb_common.c 
 gbc_arch.c 
 gb_table.c 
 gbc.c)

add_executable(${GBC_EXECUTABLE_NAME} ${GBC_SRC_LIST})

find_library(M_LIB m)
target_link_libraries(${GBC_EXECUTABLE_NAME} ${M_LIB})
target_link_libraries(${GBC_EXECUTABLE_NAME} ${CMAKE_DL_LIBS})
target_include_directories(${GBC_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_INCLUDE_DIR})
target_include_directories(${GBC_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_CONFIG_DIR})
target_include_directories(${GBC_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/share/)
target_include_directories(${GBC_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/gbc/)

# gba
set(GBA_EXECUTABLE_NAME "gba${GAMBAS_VERSION_MAJOR}")

set(GBA_SRC_LIST 
 gb_error.h gb_error.c 
 gb_alloc.c gb_array.c 
 gb_table.c 
 gb_str.h gb_str.c 
 gb_file.h gb_file.c 
 gbc_archive.h gbc_archive.c 
 gbc_chown.h gbc_chown.c 
 gb_common.c 
 gbc_arch.c 
 gba.c)

add_executable(${GBA_EXECUTABLE_NAME} ${GBA_SRC_LIST})

target_include_directories(${GBA_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_INCLUDE_DIR})
target_include_directories(${GBA_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_CONFIG_DIR})
target_include_directories(${GBA_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/share/)
target_include_directories(${GBA_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/gbc/)

# gbi
set(GBI_EXECUTABLE_NAME "gbi${GAMBAS_VERSION_MAJOR}")

set(GBI_SRC_LIST 
 gb_error.h gb_error.c 
 gb_alloc.c gb_array.c 
 gb_str.h gb_str.c 
 gb_file.h gb_file.c 
 gb_table.c 
 gb_common.c 
 gbc_arch.c 
 gbi.c)

add_executable(${GBI_EXECUTABLE_NAME} ${GBI_SRC_LIST})

target_link_libraries(${GBI_EXECUTABLE_NAME} ${CMAKE_DL_LIBS})
target_include_directories(${GBI_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_INCLUDE_DIR})
target_include_directories(${GBI_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_CONFIG_DIR})
target_include_directories(${GBI_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/share/)

target_include_directories(${GBI_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/gbc/)

# gbcm
set(GBCM_EXECUTABLE_NAME "gbcm${GAMBAS_VERSION_MAJOR}")

set(GBCM_SRC_LIST gbc_reserved_make.c)

add_executable(${GBCM_EXECUTABLE_NAME} ${GBCM_SRC_LIST})

target_include_directories(${GBCM_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_INCLUDE_DIR})
target_include_directories(${GBCM_EXECUTABLE_NAME} PUBLIC ${GAMBAS_SOURCE_CONFIG_DIR})
target_include_directories(${GBCM_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/share/)

target_include_directories(${GBCM_EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/main/gbc/)

install(TARGETS ${GBC_EXECUTABLE_NAME} ${GBA_EXECUTABLE_NAME} ${GBI_EXECUTABLE_NAME} ${GBCM_EXECUTABLE_NAME} 
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
