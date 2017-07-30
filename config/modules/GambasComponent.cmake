

###### Documentation #####
# ==== Gambas global properties ===== #
# GB_COMPONENT_LIST : A list of all encountered gambas component targets
# GB_NEED_LIST : A list of all encountered compoment implementation needs (e.g. Form,ImageIO,EventLoop, etc.)
# ==== Gambas component-related directory properties ===== #
# GB_COMPONENT_NAME : The name of the gambas native component in this directory, if any.
# ==== Gambas component-related global properties ===== #
# GB_COMPONENT_DISABLED_${_NAME} : yes if the component is disabled.
# GB_COMPONENT_SOURCE_PATH_${_NAME} : the full path of a component source directory.
# GB_COMPONENT_DEPENDENCIES_${_NAME} : a list of all the targets this component depends on.
# GB_COMPONENT_DEPENDS_${_NAME} : a list of all the targets that depends on this component.
# GB_COMPONENT_REQUIREMENTS_${_NAME} : a list of all the targets this component requires.
# GB_COMPONENT_REQUIRES_${_NAME} : a list of all the targets that requires this component.
# GB_COMPONENT_NEEDS_${_NAME} : a list of all the needs of this component.
# GB_COMPONENT_IMPLEMENTS_${_NAME} : a list of all the needs this component implements.
# GB_COMPONENT_IS_SWITCHER_${_NAME} : yes if this component is a switcher component.
# ==== Gambas need-related global properties ===== #
# GB_NEED_IMPLEMENTERS_${_NAME} : a list of all the components that implements this need.
# GB_NEED_COMPONENT_LIST_${_NAME} : a list of all the components that need this need.

#If used in standalone mode
include(GNUInstallDirs)

if(DEFINED GAMBAS_SOURCE_INCLUDE_DIR)
    set(GAMBAS_INCLUDE_DIR ${GAMBAS_SOURCE_INCLUDE_DIR})
else(DEFINED GAMBAS_SOURCE_INCLUDE_DIR)
    set(GAMBAS_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/gambas")
endif(DEFINED GAMBAS_SOURCE_INCLUDE_DIR)

macro(_gb_register_component _NAME)
    get_property(GB_COMPONENT_LIST GLOBAL PROPERTY GB_COMPONENT_LIST)
    set("GB_COMPONENT_LIST" "${GB_COMPONENT_LIST}" "${_NAME}")
    set_property(GLOBAL PROPERTY GB_COMPONENT_LIST "${GB_COMPONENT_LIST}")
endmacro()

macro(_gb_register_need _NAME)
    get_property(GB_NEED_LIST GLOBAL PROPERTY GB_NEED_LIST)
    set("GB_NEED_LIST" "${GB_NEED_LIST}" "${_NAME}")
    set_property(GLOBAL PROPERTY GB_NEED_LIST "${GB_NEED_LIST}")
endmacro()

macro(append_to_list_prop _PROPERTY _VALUE)
    get_property("${_PROPERTY}" GLOBAL PROPERTY "${_PROPERTY}")
    set("${_PROPERTY}" "${${_PROPERTY}}" "${_VALUE}")
    set_property(GLOBAL PROPERTY "${_PROPERTY}" "${${_PROPERTY}}")
    set(${_PROPERTY} "")
endmacro()

function(list_prop_exists _PROPERTY _VALUE)
    
    get_property(list GLOBAL PROPERTY "${_PROPERTY}")
#     message("Checking for ${_VALUE} in ${list}")
    foreach(i IN ITEMS ${list})
#         message("Checking ${i} STREQUAL ${_VALUE}")
        if("${i}" STREQUAL "${_VALUE}")
            set(EXISTS yes PARENT_SCOPE)
#             message("!!")
            return()
        endif()
    endforeach()
    
    set(EXISTS no PARENT_SCOPE)
    
endfunction()

function(gb_get_generated_files _COMPONENT_PATH)
    
    set(GENERATED_FILES "")
    
    
    
endfunction()

function(append_to_list_prop_unique _PROPERTY _VALUE)
    
    get_property("values" GLOBAL PROPERTY "${_PROPERTY}")
    foreach(i IN ITEMS ${values})
        if("${i}" STREQUAL "${_VALUE}")
            return()
        endif()
    endforeach()
    
    append_to_list_prop(${_PROPERTY} ${_VALUE})
    
endfunction()

function(gb_add_component_target _NAME)

    #Defining source path for this component
    set_property(GLOBAL PROPERTY "GB_COMPONENT_SOURCE_PATH_${_NAME}" "${CMAKE_CURRENT_LIST_DIR}")
    set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY "GB_COMPONENT_NAME" "${_NAME}")
    
    _gb_register_component(${_NAME})
    
    get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${_NAME}")
    if(${DISABLED})
        set(DISABLED yes PARENT_SCOPE)
        return()
    endif()
    
    add_library(${_NAME} SHARED ${ARGN})
    
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_NAME}.component")
        add_custom_command(TARGET ${_NAME}
            POST_BUILD
            COMMAND cp "${CMAKE_CURRENT_LIST_DIR}/${_NAME}.component" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.component")
    endif()
    
    get_directory_property(COMPONENT_LIBRARIES LINK_DIRECTORIES)
    if(NOT "${COMPONENT_LIBRARIES}" STREQUAL "")
        target_link_libraries(${_NAME} ${COMPONENT_LIBRARIES})
    endif()
    
    target_include_directories(${_NAME} PUBLIC 
        ${GAMBAS_INCLUDE_DIR} ${GAMBAS_SOURCE_CONFIG_DIR})
    set_target_properties(${_NAME} PROPERTIES PREFIX "")
    
    install(FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.so" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.component"
            DESTINATION "lib/gambas${GAMBAS_VERSION_MAJOR}"
            OPTIONAL)
    
    
    _gb_parse_component_file(${_NAME} "${CMAKE_CURRENT_LIST_DIR}/${_NAME}.component")
    _gb_parse_project_file(${_NAME} "${COMPONENT_SUBDIR}/.project")
    gb_check_component_depends(${_NAME})
    
endfunction()

function(gb_component)

    set(OVA NATIVE GAMBAS)
    set(MVA SOURCE_LIST SUBCOMPONENTS REQUIRED_PACKAGES)
    cmake_parse_arguments(_ "" "${OVA}" "${MVA}" ${ARGN})

    if(NOT "${__NATIVE}" STREQUAL "")
        
        foreach(package ${__REQUIRED_PACKAGES})
            gb_component_require_package("${__NATIVE}" "${package}")
        endforeach()
        
        gb_add_component("${__NATIVE}" ${__SOURCE_LIST})
        if(NOT "${__GAMBAS}" STREQUAL "")
            gb_add_subcomponent("${__NATIVE}" "${__GAMBAS}")
        endif()

        foreach(subcomponent ${__SUBCOMPONENTS})
            gb_add_subcomponent("${__NATIVE}" "${subcomponent}")
        endforeach()

    elseif(NOT "${__GAMBAS}" STREQUAL "")
        gb_add_gambas_component("${__GAMBAS}")
    endif()

endfunction()

function(gb_add_component _NAME)
    
    gb_add_component_target(${_NAME} ${ARGN})
    if(${DISABLED})
        set(DISABLED yes PARENT_SCOPE)
        return()
    endif()
    #Generating info files
    gb_generate_info_files(${_NAME} ${_NAME})
    
    
endfunction(gb_add_component)

function(gb_add_subcomponent _PARENT _NAME)
    
    if(${GB_COMPONENT_DISABLED_${_PARENT}})
        return()
    endif()
    
    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${_NAME}")
        message(FATAL_ERROR "Invalid subcomponent ${_NAME} : no valid component directory found in ${CMAKE_CURRENT_LIST_DIR}")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_NAME}/.component")
        gb_add_gambas_subcomponent(${_PARENT} ${_NAME})
    elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_NAME}/CMakeLists.txt")
        gb_add_native_subcomponent(${_PARENT} ${_NAME})
    else()
        message(FATAL_ERROR "Invalid subcomponent ${_NAME} : no valid component file found in ${CMAKE_CURRENT_LIST_DIR}/${_NAME}")
    endif()
    
#     if(TARGET ${_PARENT})
#         add_dependencies(${_NAME} ${_PARENT})
#     endif()
    
endfunction(gb_add_subcomponent)

function(gb_add_gambas_component _NAME)
    
    _gb_register_component("${_NAME}.gambas")
    
    set(GAMBAS_COMPILER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gbc${GAMBAS_VERSION_MAJOR})
    set(GAMBAS_ARCHIVER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gba${GAMBAS_VERSION_MAJOR})
    set(COMPONENT_SUBDIR "${CMAKE_CURRENT_LIST_DIR}/${_NAME}")
    
    add_custom_command(
        OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.gambas"
        COMMAND ${GAMBAS_COMPILER_PATH} ${COMPONENT_SUBDIR} -ag -r ${GAMBAS_SOURCE_OUTPUT_DIR} > /dev/null
        COMMAND ${GAMBAS_ARCHIVER_PATH} ${COMPONENT_SUBDIR} -o "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.gambas"
        COMMAND cp "${COMPONENT_SUBDIR}/.component" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.component"
        DEPENDS "${COMPONENT_SUBDIR}/.component"
        COMMENT "Building Gambas Component '${_NAME}'"
    )
    
    add_custom_target("${_NAME}.gambas" ALL
        DEPENDS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.gambas")
        
    install(FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_NAME}.gambas"
            DESTINATION "lib/gambas${GAMBAS_VERSION_MAJOR}"
            OPTIONAL)
            
    #Generating info files
    gb_generate_info_files("${_NAME}.gambas" ${_NAME})
    
    #Gambas files cleanup
    get_directory_property(ADDITIONAL_MAKE_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${ADDITIONAL_MAKE_CLEAN_FILES};${COMPONENT_SUBDIR}/.gambas")

    
    if(DEFINED GAMBAS_SOURCE_ROOT)
        add_dependencies("${_NAME}.gambas" gb gbc${GAMBAS_VERSION_MAJOR} gba${GAMBAS_VERSION_MAJOR} gbi${GAMBAS_VERSION_MAJOR})
    endif()
    
    _gb_parse_component_file(${_NAME} "${COMPONENT_SUBDIR}/.component")
    
    _gb_parse_project_file(${_NAME} "${COMPONENT_SUBDIR}/.project")
    gb_check_component_depends("${_NAME}.gambas")
    
endfunction()

function(gb_integrate_gambas_component _TARGET _COMPONENT_NAME _COMPONENT_PATH)
    
    set(GAMBAS_COMPILER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gbc${GAMBAS_VERSION_MAJOR})
    set(GAMBAS_ARCHIVER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gba${GAMBAS_VERSION_MAJOR})
    
    add_custom_command(TARGET ${_TARGET}
        POST_BUILD
        COMMAND ${GAMBAS_COMPILER_PATH} ${_COMPONENT_PATH} -ag -r ${GAMBAS_SOURCE_OUTPUT_DIR} > /dev/null
        COMMAND ${GAMBAS_ARCHIVER_PATH} ${_COMPONENT_PATH} -o "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_COMPONENT_NAME}.gambas"
        DEPENDS "${COMPONENT_SUBDIR}/.component"
        COMMENT "Integrating Gambas Component '${_COMPONENT_NAME}' for '${_TARGET}'"
    )
        
    install(FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_COMPONENT_NAME}.gambas"
            DESTINATION "lib/gambas${GAMBAS_VERSION_MAJOR}"
            OPTIONAL)
    
    #Gambas files cleanup
    get_directory_property(ADDITIONAL_MAKE_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${ADDITIONAL_MAKE_CLEAN_FILES};${COMPONENT_SUBDIR}/.gambas")

    
    if(DEFINED GAMBAS_SOURCE_ROOT)
        add_dependencies("${_TARGET}" gb gbc${GAMBAS_VERSION_MAJOR} gba${GAMBAS_VERSION_MAJOR})
    endif()
    
    _gb_parse_component_file(${_TARGET} "${_COMPONENT_PATH}/.component")
    _gb_parse_project_file(${_TARGET} "${_COMPONENT_PATH}/.project")
    
endfunction()

function(gb_add_gambas_app _NAME)
    
    _gb_register_component("${_NAME}.gambas")
    
    set(GAMBAS_COMPILER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gbc${GAMBAS_VERSION_MAJOR})
    set(GAMBAS_ARCHIVER_PATH ${GAMBAS_SOURCE_BINARY_DIR}/gba${GAMBAS_VERSION_MAJOR})
    set(COMPONENT_SUBDIR "${CMAKE_CURRENT_LIST_DIR}/${_NAME}")
    
    add_custom_command(
        OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_NAME}.gambas" "${_NAME}"
        COMMAND ${GAMBAS_COMPILER_PATH} ${COMPONENT_SUBDIR} -ag -r ${GAMBAS_SOURCE_OUTPUT_DIR} > /dev/null
        COMMAND ${GAMBAS_ARCHIVER_PATH} ${COMPONENT_SUBDIR} -o "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_NAME}.gambas"
        COMMAND ln -sf "${_NAME}.gambas" "${_NAME}"
        DEPENDS "${COMPONENT_SUBDIR}/.project"
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMENT "Building Gambas Application '${_NAME}'"
    )
    
    add_custom_target("${_NAME}.gambas" ALL
        DEPENDS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_NAME}.gambas")
        
    install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_NAME}.gambas" 
                  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_NAME}"
            PERMISSIONS WORLD_EXECUTE WORLD_READ OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
            DESTINATION "bin"
            OPTIONAL)
        
    #Generating info files
#     gb_generate_info_files("${_NAME}.gambas" ${_NAME})
    
    if(DEFINED GAMBAS_SOURCE_ROOT)
        add_dependencies("${_NAME}.gambas" gb gbc${GAMBAS_VERSION_MAJOR} gba${GAMBAS_VERSION_MAJOR} gbi${GAMBAS_VERSION_MAJOR})
    endif()
    
#     _gb_parse_component_file(${_NAME} "${COMPONENT_SUBDIR}/.component")
    _gb_parse_project_file(${_NAME} "${COMPONENT_SUBDIR}/.project")
    gb_check_component_depends("${_NAME}.gambas")
    
endfunction()

function(gb_add_gambas_subcomponent _PARENT _NAME)
    gb_add_gambas_component(${_NAME})
    gb_add_component_dependency("${_NAME}.gambas" ${_PARENT})
endfunction()

function(gb_generate_info_files _TARGET _COMPONENT)
    
    add_custom_command(TARGET ${_TARGET}
        POST_BUILD
        COMMAND mkdir -p ${GAMBAS_SOURCE_INFO_DIR}
        COMMAND "${GAMBAS_SOURCE_BINARY_DIR}/gbi${GAMBAS_VERSION_MAJOR}" ${_COMPONENT} -r ${GAMBAS_SOURCE_OUTPUT_DIR} > /dev/null
        COMMENT "Generating info files for ${_COMPONENT} ...")
        
    add_dependencies(${_TARGET} "gbi${GAMBAS_VERSION_MAJOR}")
        
    #Check if info files are already registered
    get_directory_property(ADDITIONAL_MAKE_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES)
    if(NOT "${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.info" IN_LIST ADDITIONAL_MAKE_CLEAN_FILES)
    #if(NOT "${ADDITIONAL_MAKE_CLEAN_FILES}" STREQUAL "${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.info;${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.list")
        install(FILES "${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.info" "${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.list"
            DESTINATION "share/gambas${GAMBAS_VERSION_MAJOR}/info" OPTIONAL)
        set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${ADDITIONAL_MAKE_CLEAN_FILES};${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.info;${GAMBAS_SOURCE_INFO_DIR}/${_COMPONENT}.list")
    endif()
    
endfunction()

function(gb_add_native_subcomponent _PARENT _NAME)
        add_subdirectory(${_NAME}) 
        get_property(COMPNAME DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${_NAME}" PROPERTY "GB_COMPONENT_NAME")
        gb_add_component_dependency(${COMPNAME} ${_PARENT})
endfunction()

macro(set_directory_property_from_variable _PROPERTY)
    if(${ARGC} GREATER 1)
        set(VALUES "")
        get_directory_property(VALUES ${_PROPERTY})
        set(VALUES ${VALUES} ${ARGN})
        set_directory_properties(PROPERTY ${_PROPERTY} "${VALUES}")
    endif()
endmacro()

macro(gb_component_require_package _COMPONENT _PACKAGE)
    include("Find${_PACKAGE}" OPTIONAL RESULT_VARIABLE FOUND_PACKAGE)

    if(${FOUND_PACKAGE} STREQUAL "NOTFOUND")
        message("-- Could not find CMake Module 'Find${_PACKAGE}' for component ${_COMPONENT}")
        gb_disable_component(${_COMPONENT})
    else()
        find_package(${_PACKAGE} ${ARGN})

        if(NOT ${${_PACKAGE}_FOUND})
            message("-- Could not find package '${_PACKAGE}' for component ${_COMPONENT}")
            gb_disable_component(${_COMPONENT})
        else()
            string(TOUPPER ${_PACKAGE} PACKAGE_UPPERCASE)
            set_directory_property_from_variable(LINK_DIRECTORIES ${${_PACKAGE}_LIBRARIES} ${${PACKAGE_UPPERCASE}_LIBRARIES})

            set_directory_property_from_variable(INCLUDE_DIRECTORIES ${${_PACKAGE}_INCLUDE_DIRS} ${${PACKAGE_UPPERCASE}_INCLUDE_DIRS} ${${_PACKAGE}_INCLUDE_DIR} ${${PACKAGE_UPPERCASE}_INCLUDE_DIR})
        endif()

    endif()

endmacro()

function(gb_component_require_library _COMPONENT _LIBRARY)
    
    macro(set_directory_property_from_variable _PROPERTY)
        if(${ARGC} GREATER 1)
            set(VALUES "")
            get_directory_property(VALUES ${_PROPERTY})
            set(VALUES ${VALUES} ${ARGN})
            set_directory_properties(PROPERTY ${_PROPERTY} "${VALUES}")
        endif()
    endmacro()
    
    find_library("${_LIBRARY}_LIB" ${_LIBRARY})
    
    if(NOT ${${_LIBRARY}_LIB})
        message("-- Could not find library '${_LIBRARY}' for component ${_COMPONENT}")
        gb_disable_component(${_COMPONENT})
        return()
    endif()
    
    #message("${_LIBRARY} ${LIBRARY_UPPERCASE}")
    set_directory_property_from_variable(LINK_DIRECTORIES ${${_LIBRARY}_LIB})
    
endfunction()

macro(gb_get_component_source_path _NAME)
    get_property("GB_COMPONENT_SOURCE_PATH_${_NAME}" GLOBAL PROPERTY "GB_COMPONENT_SOURCE_PATH_${_NAME}")
endmacro()

########## Gambas Component Disabling ##########


## Disables a component
## _COMPONENT : The target name (e.g. "gb.xml") of the component to disable.
function(gb_disable_component _COMPONENT)
    
    get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${_COMPONENT}")
    if(${DISABLED})
        return()
    endif()
    
    #Sets global property
    set_property(GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${_COMPONENT}" yes)
    
    #Add to disabled components list.
    get_property(GB_DISABLED_COMPONENTS GLOBAL PROPERTY GB_DISABLED_COMPONENTS)
    list(APPEND GB_DISABLED_COMPONENTS ${_COMPONENT})
    set_property(GLOBAL PROPERTY GB_DISABLED_COMPONENTS ${GB_DISABLED_COMPONENTS})
    
    #Check if target already exists. If so, then exclude it from build.
    if(TARGET ${_COMPONENT})
        set_target_properties(${_COMPONENT} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    elseif(TARGET "${_COMPONENT}.gambas")
        set_target_properties("${_COMPONENT}.gambas" PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
    
    #Check component's dependencies
    get_property("GB_COMPONENT_DEPENDS_${_COMPONENT}" GLOBAL PROPERTY "GB_COMPONENT_DEPENDS_${_COMPONENT}")
    foreach(dep IN ITEMS ${GB_COMPONENT_DEPENDS_${_COMPONENT}})
        if(TARGET ${dep})
            gb_disable_component(${dep} ${_COMPONENT})
        endif()
    endforeach()
    
endfunction()

## Returns if a given component is disabled or not.
## _COMPONENT : The target name (e.g. "gb.xml") of the component to check.
## Sets : GB_COMPONENT_DISABLED_${_COMPONENT}
macro(gb_component_is_disabled _COMPONENT)
    
    get_property("GB_COMPONENT_DISABLED_${_COMPONENT}" GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${_COMPONENT}")
    
endmacro()

########## Gambas Inter-Component Dependencies ##########

## Adds a dependency to the given component, allowing it to access shared headers.
## If the target component _DEPENDENCY does not exist yet, it will be registered later.
function(gb_add_component_dependency _COMPONENT _DEPENDENCY)
    
    if(${_COMPONENT} STREQUAL ${_DEPENDENCY})
        return()
    endif()
    
    _gb_target_is_gambas(${_COMPONENT} COMPONENT)
    _gb_target_is_gambas(${_DEPENDENCY} DEPENDENCY)
    
    #Both names got transformed to *.gambas, which means dependency is the native and component is the subcomponent gb hybrid
    if("${COMPONENT_NAME}" STREQUAL "${DEPENDENCY_NAME}")
        set(DEPENDENCY_NAME "${_DEPENDENCY}")
    endif()
    
    #Native components cannot depend on gambas ones
    #Which means that it is refering to the native one
    if(NOT ${COMPONENT_IS_GAMBAS})
        if(${DEPENDENCY_IS_GAMBAS})
            set(DEPENDENCY_NAME "${_DEPENDENCY}")
        endif()
    endif()
    
    get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${COMPONENT_NAME}")
    if(${DISABLED})
        return()
    endif()
    
    get_property(DEPENDENCIES GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${COMPONENT_NAME}")
    
    foreach(dep IN ITEMS ${DEPENDENCIES})
        if("${dep}" STREQUAL "${DEPENDENCY_NAME}")
            return()
        endif()
    endforeach()
    
    append_to_list_prop("GB_COMPONENT_DEPENDENCIES_${COMPONENT_NAME}" "${DEPENDENCY_NAME}")
    append_to_list_prop("GB_COMPONENT_DEPENDS_${DEPENDENCY_NAME}" "${COMPONENT_NAME}")
    
    #In case of circular dependency, the native component is built first.
#     list_prop_exists("GB_COMPONENT_DEPENDENCIES_${DEPENDENCY_NAME}" "${COMPONENT_NAME}")
#     set(exists2 ${EXISTS})
#     list_prop_exists("GB_COMPONENT_DEPENDENCIES_${DEPENDENCY_NAME}" "${COMPONENT_BASENAME}")
#     get_property(l GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${DEPENDENCY_NAME}")
# #     message("Checking for ${COMPONENT_NAME} in ${l} ...")
#     if(${EXISTS} OR ${exists2})
#         message("::WARNING: circular")
#         
#         if(NOT ${DEPENDENCY_IS_GAMBAS})
#             message("::Notice : Circular dependency between ${COMPONENT_NAME} and ${DEPENDENCY_NAME}.")
#             message("::Building ${COMPONENT_NAME} first.")
#             message("::removing ${COMPONENT_NAME} from ${l}")
#             list(REMOVE_ITEM "l" "${COMPONENT_NAME}" "${COMPONENT_BASENAME}")
#             set_property(GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${DEPENDENCY_NAME}" "${l}")
#             return()
#         endif()
#     endif()
#     
    if(TARGET ${DEPENDENCY_NAME})
        gb_create_target_component_dependency(${COMPONENT_NAME} ${DEPENDENCY_NAME})
    endif()
    
endfunction()

# function(gb_remove_component_dependency _COMPONENT _DEPENDENCY)
#     
# #     message("::removing ${_DEPENDENCY} from ${_COMPONENT}'s dependencies")
#     get_property(l GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${_COMPONENT}")
#     list(REMOVE_ITEM "l" "${_DEPENDENCY}")
#     set_property(GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${_COMPONENT}" "${l}")
#     
#     get_property(l GLOBAL PROPERTY "GB_COMPONENT_DEPENDS_${_DEPENDENCY}")
#     list(REMOVE_ITEM "l" "${_COMPONENT}")
#     set_property(GLOBAL PROPERTY "GB_COMPONENT_DEPENDS_${_DEPENDENCY}" "${l}")
#     
# endfunction()

function(gb_add_component_requirement _COMPONENT _REQUIREMENT)
    
    if(${_COMPONENT} STREQUAL ${_REQUIREMENT})
        return()
    endif()
    
    _gb_target_is_gambas(${_COMPONENT} COMPONENT)
    _gb_target_is_gambas(${_REQUIREMENT} REQUIREMENT)
    
    get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${COMPONENT_NAME}")
    if(${DISABLED})
        return()
    endif()
    
    get_property(REQUIREMENT GLOBAL PROPERTY "GB_COMPONENT_REQUIREMENTS_${COMPONENT_NAME}")
    
    foreach(req IN ITEMS ${REQUIREMENT})
        if("${req}" STREQUAL "${REQUIREMENT_NAME}")
            return()
        endif()
    endforeach()
    
    append_to_list_prop("GB_COMPONENT_REQUIREMENTS_${COMPONENT_NAME}" "${REQUIREMENT_NAME}")
    append_to_list_prop("GB_COMPONENT_REQUIRES_${REQUIREMENT_NAME}" "${COMPONENT_NAME}")
    
endfunction()

## Creates the dependency between two existing component targets.
function(gb_create_target_component_dependency _COMPONENT _DEPENDENCY)
    
    if("${_COMPONENT}" STREQUAL "${_DEPENDENCY}")
        return()
    endif()
    
    get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${_DEPENDENCY}")
    if(${DISABLED})
        gb_disable_component(${_COMPONENT})
        return()
    endif()
    
    _gb_target_is_gambas(${_COMPONENT} COMPONENT)
    _gb_target_is_gambas(${_DEPENDENCY} DEPENDENCY)
    
    #Both names got transformed to *.gambas, which means dependency is the native and component is the subcomponent gb hybrid
    if("${COMPONENT_NAME}" STREQUAL "${DEPENDENCY_NAME}")
        set(DEPENDENCY_NAME "${_DEPENDENCY}")
    endif()
    
    #Native components cannot depend on gambas ones
    #Which means that it is refering to the native one
    if(NOT ${COMPONENT_IS_GAMBAS})
        if("${DEPENDENCY_IS_GAMBAS}")
            set(DEPENDENCY_NAME "${_DEPENDENCY}")
            set(DEPENDENCY_IS_GAMBAS NO)
        endif()
    endif()
    
    get_property(isswitcher GLOBAL PROPERTY "GB_COMPONENT_IS_SWITCHER_${_DEPENDENCY}")
    if(${isswitcher})
        get_property(imps GLOBAL PROPERTY "GB_COMPONENT_IMPLEMENTS_${_DEPENDENCY}")
    
        foreach(imp IN ITEMS ${imps})
            gb_add_component_need(${_COMPONENT} ${imp})
        endforeach()
        #return()
    endif()
    
    # message("#Adding ${DEPENDENCY_NAME} as a dependency of ${COMPONENT_NAME}")
    add_dependencies(${COMPONENT_NAME} "${DEPENDENCY_NAME}")
    
    
    if(NOT ${DEPENDENCY_IS_GAMBAS})
        if(NOT ${COMPONENT_IS_GAMBAS})
            gb_get_component_source_path(${_DEPENDENCY})
                target_include_directories(${_COMPONENT} PUBLIC 
                    "${GB_COMPONENT_SOURCE_PATH_${_DEPENDENCY}}")
        endif()
    endif()

endfunction()

## Retroactively 
function(gb_check_component_depends _COMPONENT)
    _gb_target_is_gambas("${_COMPONENT}" COMPONENT)
#     message("=Post-checking ${COMPONENT_NAME}")
    
    get_property(deps GLOBAL PROPERTY "GB_COMPONENT_DEPENDS_${COMPONENT_NAME}")
    
    #If the component was gambas, it couldn't be declared as ".gambas" before target creation, and not added in the right property.
    if(${COMPONENT_IS_GAMBAS})
        get_property(deps2 GLOBAL PROPERTY "GB_COMPONENT_DEPENDS_${COMPONENT_BASENAME}")
        set(deps ${deps} ${deps2})
    endif()
    
    foreach(dep IN ITEMS ${deps})
        _gb_target_is_gambas("${dep}" DEPENDENCY)
        if(TARGET ${DEPENDENCY_NAME})
            gb_create_target_component_dependency(${DEPENDENCY_NAME} ${COMPONENT_NAME})
        endif()
    endforeach()
    
endfunction()

function(gb_check_dependencies)
    get_property(GB_COMPONENT_LIST GLOBAL PROPERTY GB_COMPONENT_LIST)
    _gb_check_requirements()
    foreach(comp IN ITEMS ${GB_COMPONENT_LIST})
        get_property(GB_COMPONENT_DEPENDENCIES_${comp} GLOBAL PROPERTY GB_COMPONENT_DEPENDENCIES_${comp})
        foreach(dep IN ITEMS ${GB_COMPONENT_DEPENDENCIES_${comp}})
            if(NOT TARGET ${dep})
                if(NOT TARGET "${dep}.gambas")
                    message("!! WARNING : Dependency ${dep} not found for ${comp}.")
                    gb_disable_component(${comp})
                endif()
            endif()
        endforeach()
    endforeach()
    
endfunction()

function(_gb_check_requirements)
    
    macro(add_requirement _COMPONENT)
    
        get_property(isswitcher GLOBAL PROPERTY "GB_COMPONENT_IS_SWITCHER_${_COMPONENT}")
        
        if(NOT "${isswitcher}")
            get_property(deplist GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${_COMPONENT}")
            foreach(dep IN ITEMS ${deplist})
                get_property(reqlist GLOBAL PROPERTY "GB_COMPONENT_REQUIREMENTS_${dep}")
                foreach(req IN ITEMS ${reqlist})
                    gb_add_component_dependency(${_COMPONENT} ${req})
                endforeach()
            endforeach()
        endif()
        
#         if("${_REQUIREMENT}" STREQUAL "${_COMPONENT}")
#             return()
#         endif()
#         
#         if("${_REQUIREMENT}" STREQUAL "")
#             get_property(reqlist GLOBAL PROPERTY "GB_COMPONENT_DEPENDENCIES_${_COMPONENT}")
#         else()
#             get_property(reqlist GLOBAL PROPERTY "GB_COMPONENT_REQUIREMENTS_${_REQUIREMENT}")
#             list_prop_exists("GB_COMPONENT_DEPENDENCIES_${_COMPONENT}" "${_REQUIREMENT}")
#             if("${EXISTS}")
#                 message("${_REQUIREMENT} exists already for ${_COMPONENT} !!")
#                 return()
#             endif()
#             message("Adding requirement ${_REQUIREMENT} to ${_COMPONENT}")
#             gb_add_component_dependency(${_COMPONENT} ${_REQUIREMENT})
#         endif()
#         
#         foreach(req IN ITEMS ${reqlist})
#             add_requirement("${_COMPONENT}" "${req}")
#         endforeach()
        
    endmacro()
    
    get_property(components GLOBAL PROPERTY "GB_COMPONENT_LIST")
    foreach(comp IN ITEMS ${components})
        add_requirement("${comp}")
    endforeach()
            
    
endfunction()

function(_gb_parse_component_file _COMPONENT _FILE)
    
    macro(read_list_line _TEXT _VAR)
        string (REGEX MATCHALL "(^|;)${_TEXT}=[^;]*" ${_TEXT} "${_VAR}")
        if(NOT "${${_TEXT}}" STREQUAL "")
            string(LENGTH "${_TEXT}" textlen)
            string(SUBSTRING "${${_TEXT}}" ${textlen} -1 ${_TEXT})
            string(SUBSTRING "${${_TEXT}}" 2 -1 ${_TEXT})
            string(REPLACE "," ";" ${_TEXT} "${${_TEXT}}")
        else()
            set(${_TEXT} "")
        endif()
    endmacro()
    
    if(NOT EXISTS ${_FILE})
        return()
    endif()
    
    file(READ "${_FILE}" RAW_CONTENTS)
    
    string(REPLACE "\n" ";" LINES "${RAW_CONTENTS}")
    
    read_list_line(Requires "${LINES}")
    foreach(dep IN ITEMS ${Requires})
            gb_add_component_requirement(${_COMPONENT} ${dep})
    endforeach()
    
    #BUG in gb.qt4.OpenGL ?
    read_list_line(Require "${LINES}")
    foreach(dep IN ITEMS ${Require})
            gb_add_component_requirement(${_COMPONENT} ${dep})
    endforeach()
    
    read_list_line(Implements "${LINES}")
    foreach(imp IN ITEMS ${Implements})
            gb_component_implements(${_COMPONENT} ${imp})
    endforeach()
    
    read_list_line(Needs "${LINES}")
    foreach(need IN ITEMS ${Needs})
            gb_add_component_need(${_COMPONENT} ${need})
    endforeach()
    
    #BUG in OpenGL ?
    read_list_line(Need "${LINES}")
    foreach(need IN ITEMS ${Need})
            gb_add_component_need(${_COMPONENT} ${need})
    endforeach()
    
endfunction()

function(_gb_parse_project_file _COMPONENT _FILE)
    
#     message("Parsing project file '${_FILE}' for ${COMPONENT_NAME}")
    if(NOT EXISTS ${_FILE})
        return()
    endif()
    
    _gb_target_is_gambas(${_COMPONENT} COMPONENT)
    
    
    file(READ "${_FILE}" RAW_CONTENTS)
    
    string(REPLACE "\n" ";" LINES "${RAW_CONTENTS}")
    
    string (REGEX MATCHALL "(^|;)Component=[^;]*" Components "${LINES}")
    foreach(comp IN ITEMS ${Components})
        string(SUBSTRING "${comp}" 10 -1 comp)
        gb_add_component_dependency("${COMPONENT_NAME}" ${comp})
    endforeach()
    
    
endfunction()

function(_gb_target_is_gambas _TARGET _OUTPUTNAME)
    
    #Name can contain ".gambas", it is at the end of the name and is a valid target
    if("${_TARGET}" MATCHES "\\.gambas$" AND TARGET "${_TARGET}")
        set(${_OUTPUTNAME}_IS_GAMBAS yes PARENT_SCOPE)
        set(${_OUTPUTNAME}_NAME ${_TARGET} PARENT_SCOPE)
        
        #Calculating raw name
        string(LENGTH "${_TARGET}" len)
        math(EXPR len "${len} - 7")
        string(SUBSTRING "${_TARGET}" 0 ${len} basename)
        set(${_OUTPUTNAME}_BASENAME ${basename} PARENT_SCOPE)
    elseif(TARGET "${_TARGET}.gambas")#NAME.gambas is a valid gambas target already > yes.
        set(${_OUTPUTNAME}_IS_GAMBAS yes PARENT_SCOPE)
        set(${_OUTPUTNAME}_NAME "${_TARGET}.gambas" PARENT_SCOPE)
        set(${_OUTPUTNAME}_BASENAME ${_TARGET} PARENT_SCOPE)
    elseif(TARGET "${_TARGET}")#NAME is a valid native target already > no.
        set(${_OUTPUTNAME}_IS_GAMBAS no PARENT_SCOPE)
        set(${_OUTPUTNAME}_NAME ${_TARGET} PARENT_SCOPE)
        set(${_OUTPUTNAME}_BASENAME ${_TARGET} PARENT_SCOPE)
    else()
        set(${_OUTPUTNAME}_IS_GAMBAS no PARENT_SCOPE)
        set(${_OUTPUTNAME}_NAME ${_TARGET} PARENT_SCOPE)
        set(${_OUTPUTNAME}_BASENAME ${_TARGET} PARENT_SCOPE)
    endif()
    
endfunction()

########## Gambas Needs Management ##########

# Checks if a given Need exists.
# _NAME : The name of the Need to check.
# Sets : NEED_EXISTS : yes if the needs exists, no otherwise
function(gb_need_exists _NAME)
    get_property(GB_NEED_LIST GLOBAL PROPERTY GB_NEED_LIST)
    foreach(need IN ITEMS ${GB_NEED_LIST})
        if("${need}" STREQUAL "${_NAME}")
            set(NEED_EXISTS yes PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(NEED_EXISTS no PARENT_SCOPE)
endfunction()

function(_gb_create_need _NAME)
    gb_need_exists("${_NAME}")
    if(${NEED_EXISTS})
        return()
    endif()
    
    _gb_register_need("${_NAME}")
    
endfunction()

function(gb_add_component_need _COMPONENT _NEED)
    _gb_create_need(${_NEED})
    append_to_list_prop("GB_COMPONENT_NEEDS_${_COMPONENT}" "${_NEED}")
    append_to_list_prop("GB_NEED_COMPONENT_LIST_${_NEED}" "${_COMPONENT}")
endfunction()

function(gb_component_implements _COMPONENT)
    append_to_list_prop("GB_COMPONENT_IMPLEMENTS_${_COMPONENT}" "${ARGN}")
    foreach(need IN ITEMS ${ARGN})
        _gb_create_need(${need})
        append_to_list_prop("GB_NEED_IMPLEMENTERS_${need}" "${_COMPONENT}")
    endforeach()
endfunction()

function(gb_component_set_switcher _COMPONENT)
    set_property(GLOBAL PROPERTY "GB_COMPONENT_IS_SWITCHER_${_COMPONENT}" "yes")
endfunction()

function(gb_component_register_switcher _COMPONENT _SWITCHER)
    gb_generate_info_files(${_COMPONENT} ${_SWITCHER})
endfunction()

function(gb_get_need_implementer _NEED)
    
    get_property(implementers GLOBAL PROPERTY GB_NEED_IMPLEMENTERS_${_NEED})
    foreach(imp IN ITEMS ${implementers})
        get_property(isswitcher GLOBAL PROPERTY "GB_COMPONENT_IS_SWITCHER_${imp}")
        if(NOT "${isswitcher}")
            get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${imp}")
            if(NOT "${DISABLED}")
                set(IMPLEMENTER "${imp}" PARENT_SCOPE)
                return()
            endif()
        endif()
    endforeach()
    
    set(IMPLEMENTER "" PARENT_SCOPE)
    
endfunction()

function(_gb_disable_needers _NEED)
    
    get_property(GB_NEED_COMPONENT_LIST_${need} GLOBAL PROPERTY GB_NEED_COMPONENT_LIST_${need})
    foreach(comp IN ITEMS ${GB_NEED_COMPONENT_LIST_${need}})
        gb_disable_component(${comp})
    endforeach()
    
endfunction()

function(gb_check_needs)
    get_property(GB_NEED_LIST GLOBAL PROPERTY GB_NEED_LIST)
    
    foreach(need IN ITEMS ${GB_NEED_LIST})
        
        gb_get_need_implementer(${need})
        if(NOT "${IMPLEMENTER}" STREQUAL "")
            get_property(GB_NEED_COMPONENT_LIST_${need} GLOBAL PROPERTY GB_NEED_COMPONENT_LIST_${need})
            foreach(comp IN ITEMS ${GB_NEED_COMPONENT_LIST_${need}})
                get_property(DISABLED GLOBAL PROPERTY "GB_COMPONENT_DISABLED_${comp}")
                if(NOT "${DISABLED}")
                    gb_add_component_requirement(${comp} ${IMPLEMENTER})
                    gb_add_component_dependency(${comp} ${IMPLEMENTER})
                endif()
            endforeach()
        else()
            message("Warning : no implementer found for need ${need}")
            _gb_disable_needers(${need})
        endif()
    endforeach()
    
endfunction()
