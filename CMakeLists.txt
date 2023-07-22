cmake_minimum_required(VERSION 3.18.4 FATAL_ERROR)

project(parr
    VERSION   0.1
    LANGUAGES CXX
)

set(PARR_BIN  ${PROJECT_NAME})
set(PARR_LIB  ${PARR_BIN}lib)
set(PARR_SSL  ${PARR_BIN}ssl)
set(PARR_ABNF ${PARR_BIN}abnf)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  YES)
set(CMAKE_SKIP_INSTALL_RULES     YES)

macro(install)
endmacro()
macro(export)
endmacro()

# Has to done very early if CMake directory is submodule
if(EXISTS "${PROJECT_SOURCE_DIR}/.git/HEAD" AND NOT EXISTS "${PROJECT_SOURCE_DIR}/.git/modules")
    find_package(Git QUIET)
    if(Git_FOUND)
        message(STATUS "Initializing git submodules")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive)
    endif()
endif()

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
    parr_build_option(${target} "compile" UNIX FALSE -Os                    PARR_COMPILE_FLAG_Os)

    parr_build_option(${target} "compile" MSVC TRUE  /options:strict        PARR_COMPILE_FLAG_options_strict)
    parr_build_option(${target} "compile" MSVC TRUE  /permissive-           PARR_COMPILE_FLAG_permissive_)
    parr_build_option(${target} "compile" MSVC FALSE /Os                    PARR_COMPILE_FLAG_Os)
    parr_build_option(${target} "compile" MSVC FALSE /bigobj                PARR_COMPILE_FLAG_bigobj)

    parr_build_option(${target} "compile" MSVC TRUE  /options:strict        PARR_COMPILE_FLAG_options_strict)
    parr_build_option(${target} "compile" MSVC TRUE  /permissive-           PARR_COMPILE_FLAG_permissive_)
    parr_build_option(${target} "compile" MSVC FALSE /Ot                    PARR_COMPILE_FLAG_Ot)
    parr_build_option(${target} "compile" MSVC FALSE /bigobj                PARR_COMPILE_FLAG_bigobj)

    parr_build_option(${target} "compile" UNIX TRUE  -Wall                  PARR_COMPILE_FLAG_Wall)
    parr_build_option(${target} "compile" UNIX TRUE  -Wextra                PARR_COMPILE_FLAG_Wextra)
    parr_build_option(${target} "compile" UNIX TRUE  -Wpedantic             PARR_COMPILE_FLAG_Wpedantic)

    parr_build_option(${target} "compile" MSVC TRUE  /W4                    PARR_COMPILE_FLAG_W4)

    parr_build_option(${target} "compile" UNIX FALSE -Wdate-time            PARR_COMPILE_FLAG_Wdate_time)
    parr_build_option(${target} "compile" UNIX FALSE -Weffc++               PARR_COMPILE_FLAG_Weffcxx)
    parr_build_option(${target} "compile" UNIX FALSE -Wmissing-include-dirs PARR_COMPILE_FLAG_Wmissing_include_dirs)
    parr_build_option(${target} "compile" UNIX FALSE -Wold-style-cast       PARR_COMPILE_FLAG_Wold_style_cast)
    parr_build_option(${target} "compile" UNIX FALSE -Wshadow               PARR_COMPILE_FLAG_Wshadow)
    parr_build_option(${target} "compile" UNIX FALSE -Wunused-macros        PARR_COMPILE_FLAG_Wunused_macros)
    parr_build_option(${target} "compile" UNIX FALSE -Wuseless-cast         PARR_COMPILE_FLAG_Wuseless-cast)

    parr_build_option(${target} "compile" UNIX FALSE -Werror                PARR_COMPILE_FLAG_Werror)

    parr_build_option(${target} "link"    MSVC FALSE /OPT:REF               PARR_COMPILE_FLAG_OPT_REF)
endforeach()

#
#
#

add_subdirectory(Test)

#

if(NOT "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" AND NOT EXISTS .gitignore)
    file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()
