cmake_minimum_required(VERSION 3.18.4 FATAL_ERROR)

project(parr
    VERSION   0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  YES)
set(CMAKE_CXX_EXTENSIONS         NO)
set(CMAKE_SKIP_INSTALL_RULES     YES)

set(PARR_BIN  ${PROJECT_NAME})
set(PARR_LIB  ${PARR_BIN}lib)
set(PARR_SSL  ${PARR_BIN}ssl)
set(PARR_ABNF ${PARR_BIN}abnf)

macro(install)
endmacro()
macro(export)
endmacro()

# Has to be done very early if CMake directory is submodule
if(EXISTS "${PROJECT_SOURCE_DIR}/.git/HEAD" AND NOT EXISTS "${PROJECT_SOURCE_DIR}/.git/modules")
    find_package(Git QUIET)
    if(Git_FOUND)
        message(STATUS "Initializing git submodules")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive)
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
include(BuildOption)

#
# External libraries
# All libraries must be configured in subdirectory
#

add_subdirectory(Libs)

#
# Project targets
#

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
        ssl/SSL.abnf.hpp
)
target_compile_definitions(${PARR_LIB} PRIVATE PROJECT_VERSION=${PROJECT_VERSION} PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} PROJECT_VERSION_TWEAK=${PROJECT_VERSION_TWEAK})
target_include_directories(${PARR_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(${PARR_LIB} PUBLIC pegtl)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf.hpp
    COMMENT "Generating SSL.abnf header"
    COMMAND "$<TARGET_FILE:${PARR_ABNF}>" ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf > ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf.hpp
    DEPENDS ${PARR_ABNF} ${PROJECT_SOURCE_DIR}/ssl/SSL.abnf
)

function(parr_executable target)
    add_executable(${target} "")
    target_sources(${target}
        PRIVATE
            ${CMAKE_CURRENT_LISTS_FILE}

            executable/${target}.cpp
    )
endfunction()

parr_executable(${PARR_ABNF})
target_link_libraries(${PARR_ABNF} PRIVATE pegtl)

parr_executable(${PARR_SSL})
target_link_libraries(${PARR_SSL} PRIVATE ${PARR_LIB} cxxopts)

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically/
# https://learn.microsoft.com/en-us/cpp/build/reference/linker-options/
get_property(PARR_TARGETS DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(target IN ITEMS ${PARR_TARGETS})
    message(STATUS "Configure ${target} ...")
    # general

    project_build_option(${target} UNIX COMPILE CXX FALSE -Os)

    project_build_option(${target} MSVC COMPILE CXX TRUE  /options:strict)
    project_build_option(${target} MSVC COMPILE CXX TRUE  /permissive-)
    project_build_option(${target} MSVC COMPILE CXX FALSE /Os)
    project_build_option(${target} MSVC COMPILE CXX FALSE /bigobj)

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

    # misc

    project_build_option(${target} MSVC COMPILE CXX FALSE /MT)
    project_build_option(${target} MSVC LINKING CXX FALSE /OPT:REF)
endforeach()

#
# Generate targets running tests
# While they are always generated, they must be started manually
#

add_subdirectory(Test)

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
        COMMENT "Copying artifacts: $ENV{ARTIFACTS_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_LIST_DIR}/$ENV{ARTIFACTS_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${PARR_SSL}>" "${CMAKE_CURRENT_LIST_DIR}/$ENV{ARTIFACTS_DIR}"
    )
endif()
