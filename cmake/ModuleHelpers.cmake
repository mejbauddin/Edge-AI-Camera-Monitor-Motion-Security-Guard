function(csx_add_library target)
    set(options "")
    set(oneValueArgs SRC_DIR)
    set(multiValueArgs DEPS QT_COMPONENTS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE _sources
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SRC_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SRC_DIR}/*.hpp"
    )

    if(_sources)
        add_library(${target} STATIC ${_sources})
    else()
        add_library(${target} INTERFACE)
    endif()

    if(_sources)
        target_include_directories(${target}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SRC_DIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                $<INSTALL_INTERFACE:include>
        )
    else()
        target_include_directories(${target}
            INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SRC_DIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                $<INSTALL_INTERFACE:include>
        )
    endif()

    if(ARG_DEPS)
        if(_sources)
            target_link_libraries(${target} PUBLIC ${ARG_DEPS})
        else()
            target_link_libraries(${target} INTERFACE ${ARG_DEPS})
        endif()
    endif()

    if(ARG_QT_COMPONENTS AND _sources)
        csx_link_qt(${target} COMPONENTS ${ARG_QT_COMPONENTS})
    endif()

    if(_sources)
        csx_set_target_warnings(${target})
        target_compile_features(${target} PUBLIC cxx_std_20)
    endif()
endfunction()

function(csx_link_qt target)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs COMPONENTS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_package(Qt6 REQUIRED COMPONENTS ${ARG_COMPONENTS})
    foreach(_component IN LISTS ARG_COMPONENTS)
        string(TOUPPER ${_component} _upper)
        target_link_libraries(${target} PRIVATE Qt6::${_component})
    endforeach()
endfunction()
