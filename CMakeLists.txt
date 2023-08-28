cmake_minimum_required(VERSION 3.18.4 FATAL_ERROR)

set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(prs
    VERSION   0.2
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE             Release)
set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  YES)
set(CMAKE_CXX_EXTENSIONS         NO)
set(CMAKE_SKIP_INSTALL_RULES     YES)

set(PRS_LIB     ${PROJECT_NAME})
set(PRS_LIB_BIN ${PRS_LIB}.executable)
set(PRS_LIB_SSL ${PRS_LIB}.ssl)

set(PRS_BIN           ${PROJECT_NAME})
set(PRS_BIN_PROCESSOR ${PRS_BIN}-processor)
set(PRS_BIN_SSL       ${PRS_BIN}-ssl)

macro(install)
endmacro()
macro(export)
endmacro()

# Has to be done very early if one of CMake directories is submodule
if(EXISTS "${PROJECT_SOURCE_DIR}/.git/HEAD" AND NOT EXISTS "${PROJECT_SOURCE_DIR}/.git/modules")
    find_package(Git QUIET)
    if(Git_FOUND)
        message(STATUS "Initializing git submodules")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive)
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
include(ANTLR)
include(BuildOption)

#
# External libraries
# All libraries must be configured in subdirectory
#

add_subdirectory(Libs)

#
# Project targets
#

set(PRS_ANTLR_URL "https://github.com/antlr/website-antlr4/raw/gh-pages/download/antlr-4.13.0-complete.jar")
set(PRS_ANTLR_DIR "${PROJECT_BINARY_DIR}/ANTLR")
set(PRS_ANTLR_JAR "${PRS_ANTLR_DIR}/antlr.jar")

project_antlr_download("${PRS_ANTLR_URL}" "${PRS_ANTLR_JAR}")
project_antlr_library(${PROJECT_NAME} "processor" "Processor" "${PRS_ANTLR_JAR}") # -> PRS_LIB_PROCESSOR
project_antlr_library(${PROJECT_NAME} "ssl" "FalloutScript" "${PRS_ANTLR_JAR}")   # -> PRS_LIB_SSL

#

add_library(${PRS_LIB} STATIC)
target_sources(${PRS_LIB}
    PRIVATE
        "${CMAKE_CURRENT_LIST_FILE}"

        Source/prs.cpp
        Source/prs.hpp
)
target_compile_definitions(${PRS_LIB} PRIVATE PROJECT_VERSION=${PROJECT_VERSION} PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} PROJECT_VERSION_TWEAK=${PROJECT_VERSION_TWEAK})
target_include_directories(${PRS_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Source")
target_link_libraries(${PRS_LIB} PUBLIC ${PROJECT_NAME}+antlr)

add_library(${PRS_LIB_BIN} STATIC)
target_sources(${PRS_LIB_BIN}
    PRIVATE
        "${CMAKE_CURRENT_LIST_FILE}"

        Source/executable/executable.cpp
        Source/executable/executable.cpp
)
target_include_directories(${PRS_LIB_BIN} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Source/executable")
target_link_libraries(${PRS_LIB_BIN} PUBLIC ${PRS_LIB} cxxopts)

####

#add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf.hpp
#    COMMENT "Generating SSL.abnf header"
#    COMMAND "$<TARGET_FILE:${PRS_ABNF}>" ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf > ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf.hpp
#    DEPENDS ${PRS_ABNF} ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf
#)

function(prs_executable target lib)
    if( NOT EXISTS "${PROJECT_SOURCE_DIR}/Source/executable/${target}.cpp" )
        message( AUTHOR_WARNING "Main source for target \"${target}\" does not exist\n${PROJECT_SOURCE_DIR}/Source/executable/${target}.cpp" )
        return()
    endif()

    add_executable(${target} "")
    target_sources(${target}
        PRIVATE
            "${CMAKE_CURRENT_LISTS_FILE}"

             Source/executable/${target}.cpp

            "${PRS_ANTLR_DIR}/${lib}/cpp/${PROJECT_NAME}.${lib}.hpp"
    )
    target_link_libraries(${target} PRIVATE ${PRS_LIB_BIN} ${PROJECT_NAME}.${lib})
endfunction()

prs_executable(${PRS_BIN_PROCESSOR} processor)
prs_executable(${PRS_BIN_SSL} ssl)

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically/
# https://learn.microsoft.com/en-us/cpp/build/reference/linker-options/
get_property(PRS_TARGETS DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(target IN ITEMS ${PRS_TARGETS})
    message(STATUS "Configuring build options: ${target}" )

    # general

    project_build_option(${target} UNIX COMPILE CXX FALSE -Os)

    project_build_option(${target} MSVC COMPILE CXX TRUE  /options:strict)
    project_build_option(${target} MSVC COMPILE CXX TRUE  /permissive-)
    project_build_option(${target} MSVC COMPILE CXX FALSE /Os)
    project_build_option(${target} MSVC COMPILE CXX FALSE /bigobj)

    # misc

    project_build_option(${target} MSVC COMPILE CXX FALSE /MT)
    project_build_option(${target} MSVC LINKING CXX FALSE /OPT:REF)

    # cygwin

    if(CYGWIN)
        project_build_option(${target} TRUE LINKING CXX TRUE -static)
        project_build_option(${target} TRUE LINKING CXX TRUE -static-libgcc)
        project_build_option(${target} TRUE LINKING CXX TRUE -static-libstdc++)
    endif()

    # skip flags below for targets with generated code
    get_target_property(tmp ${target} ANTLR_LIBRARY)
    if( ${tmp} )
        continue()
    endif()

    # warnings

    project_build_option(${target} UNIX COMPILE CXX TRUE  -Wall)
    project_build_option(${target} UNIX COMPILE CXX TRUE  -Wextra)
    project_build_option(${target} UNIX COMPILE CXX TRUE  -Wpedantic)

    project_build_option(${target} MSVC COMPILE CXX TRUE  /W4)

    project_build_option(${target} UNIX COMPILE CXX FALSE -Wdate-time)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Weffc++)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Wmissing-include-dirs)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Wold-style-cast)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Wshadow)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Wunused-macros)
    project_build_option(${target} UNIX COMPILE CXX FALSE -Wuseless-cast)

    project_build_option(${target} UNIX COMPILE CXX FALSE -Werror)

endforeach()

#
# Generate targets running tests
# While they are always generated, they must be started manually
#

add_subdirectory(Test)

#
# cygwin1.dll
# Static executables still require .dll to run outside of Cygwin shell
#

if(CYGWIN)
    find_program(PRS_CYGWIN1_PATH cygwin1.dll REQUIRED)
    get_filename_component(PRS_CYGWIN1_NAME "${PRS_CYGWIN1_PATH}" NAME)
    if(NOT EXISTS "${PROJECT_BINARY_DIR}/${PRS_CYGWIN1_NAME}")
        message(STATUS "Copying ${PRS_CYGWIN1_NAME}")
        file(COPY "${PRS_CYGWIN1_PATH}" DESTINATION "${PROJECT_BINARY_DIR}")
    endif()
endif()

#
# Add build directory to git ignores list
#

if(NOT "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" AND NOT EXISTS .gitignore)
    file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

#
# Copy selected targets to different directory
# Used with GitHub Actions
#

if(DEFINED ENV{ARTIFACTS_DIR} AND NOT "$ENV{ARTIFACTS_DIR}" MATCHES "^(|[0-9]+|[Oo][Nn]|[Yy][Ee][Ss]|[Tt][Rr][Uu][Ee]|[Yy]|[Oo][Ff][Ff]|[Nn][Oo]|[Ff][Aa][Ll][Ss][Ee]|[Nn])$" )
    message(STATUS "Artifacts directory: $ENV{ARTIFACTS_DIR}")
    add_custom_target(artifacts-dir
        ALL
        COMMENT "Copying artifacts: ${PRS_BIN_SSL} -> $ENV{ARTIFACTS_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_LIST_DIR}/$ENV{ARTIFACTS_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${PRS_BIN_SSL}>" "${CMAKE_CURRENT_LIST_DIR}/$ENV{ARTIFACTS_DIR}"
    )
endif()
