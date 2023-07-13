cmake_minimum_required(VERSION 3.18.4 FATAL_ERROR)

project(parr
    LANGUAGES CXX
)

set(PARR_BIN ${PROJECT_NAME})
set(PARR_LIB ${PARR_BIN}lib)
set(PARR_SSL ${PARR_BIN}ssl)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  YES)
set(CMAKE_SKIP_INSTALL_RULES     YES)

macro(install)
endmacro()
macro(export)
endmacro()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
include(BuildOption)

add_subdirectory(Libs)

add_library(${PARR_LIB} STATIC)
target_sources(${PARR_LIB}
    PRIVATE
        ${CMAKE_CURRENT_LIST_FILE}

        StaticAssert.cpp

        Parser.cpp
        Parser.hpp
        Parser.State.cpp

        Rules.hpp

        ssl/SSL.cpp
        ssl/SSL.hpp
        ssl/SSL.Rules.hpp
)
#target_include_directories(${PARR_LIB} PRIVATE ${PARR_INCLUDE_FMT})
target_include_directories(${PARR_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(${PARR_LIB} PUBLIC pegtl)

add_executable(${PARR_SSL} "")
target_sources(${PARR_SSL}
    PRIVATE
        ${CMAKE_CURRENT_LISTS_FILE}

        executable/${PARR_SSL}.cpp
)
target_link_libraries(${PARR_SSL} PRIVATE ${PARR_LIB} cxxopts)

get_property(PARR_TARGETS DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(target IN ITEMS ${PARR_TARGETS})
    message(STATUS "Configure ${target} ...")
    parr_build_option(${target} "compile" UNIX FALSE -O2                    PARR_COMPILE_FLAG_O2)

    parr_build_option(${target} "compile" UNIX TRUE  -Wall                  PARR_COMPILE_FLAG_Wall)
    parr_build_option(${target} "compile" UNIX FALSE -Wextra                PARR_COMPILE_FLAG_Wextra)
    parr_build_option(${target} "compile" UNIX FALSE -Wpedantic             PARR_COMPILE_FLAG_Wpedantic)

    parr_build_option(${target} "compile" UNIX FALSE -Wdate-time            PARR_COMPILE_FLAG_Wdate_time)
    parr_build_option(${target} "compile" UNIX FALSE -Wmissing-include-dirs PARR_COMPILE_FLAG_Wmissing_include_dirs)
    parr_build_option(${target} "compile" UNIX FALSE -Wold-style-cast       PARR_COMPILE_FLAG_Wold_style_cast)
    parr_build_option(${target} "compile" UNIX FALSE -Wshadow               PARR_COMPILE_FLAG_Wshadow)
    parr_build_option(${target} "compile" UNIX FALSE -Wunused-macros        PARR_COMPILE_FLAG_Wunused_macros)
    parr_build_option(${target} "compile" UNIX FALSE -Wuseless-cast         PARR_COMPILE_FLAG_Wuseless-cast)

    parr_build_option(${target} "compile" UNIX FALSE -Werror                PARR_COMPILE_FLAG_Werror)
endforeach()

#
#
#

add_subdirectory(Test)

#

if(NOT "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" AND NOT EXISTS .gitignore)
    file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()
