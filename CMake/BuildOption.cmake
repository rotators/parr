include_guard()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

# project_build_option( target guard type lang required flag )
#   target ...... name of the the target
#   guard .....,. name of variable used to test if given flag should be tested
#                 if variable resolves to FALSE, function does nothing
#   type ........ string describing which CMake functions should be used to test and add <flag> to <target>
#                 COMPILE ... test: check_c_compiler_flag() / check_cxx_compiler_flag  add: target_compile_options()
#                 LINKING ... test: check_linker flag(C/CXX)                           add: target_link_options()
#   required .... name of variable used to test if given flag is mandatory
#                 if flag is not found and variable resolves to TRUE, configuration stops with error message
#   flag......... see CMake documentation
#                 https://cmake.org/cmake/help/v3.18/module/CheckCCompilerFlag.html
#                 https://cmake.org/cmake/help/v3.18/module/CheckCXXCompilerFlag.html
#                 https://cmake.org/cmake/help/v3.18/module/CheckLinkerFlag.html

function(project_build_option target guard type lang required flag)
    if(NOT ${guard})
        return()
    endif()

    # prepare variable name

    set(flagvar "${flag}")
    string(REGEX REPLACE "^[/-]" "" flagvar "${flag}")
    set(flagvar "_${PROJECT_NAME}__${flagvar}")

    foreach(what IN ITEMS "++")
        string(REPLACE ${what} "xx" flagvar "${flagvar}")
    endforeach()

    string(MAKE_C_IDENTIFIER "${flagvar}" flagvar)

    # check arguments

    if("${lang}" STREQUAL "C" OR "${lang}" STREQUAL "CXX")
        set(flagvar "${lang}_${flagvar}")
    else()
        message(AUTHOR_WARNING "Unknown flag language: ${lang}")
        message(AUTHOR_WARNING "Valid flag languages:  C, CXX")
        return()
    endif()

    if("${type}" STREQUAL "COMPILE" OR "${type}" STREQUAL "LINKING")
        set(flagvar "${type}_${flagvar}")
    else()
        message(AUTHOR_WARNING "Unknown flag type: ${type}")
        message(AUTHOR_WARNING "Valid flag types:  COMPILE, LINKING")
        return()
    endif()

    set(flagvar "BUILD_OPTION_${flagvar}")
    #message(STATUS "flagvar ${flagvar}")

    # assert

    if(NOT "${flagvar}" MATCHES "^[A-Za-z0-9_]+$")
        message(FATAL_ERROR "Invalid flag variable: ${flagvar}")
        return()
    endif()

    # actual test finally

    if("${type}" STREQUAL "COMPILE")
        if("${lang}" STREQUAL "C")
            check_c_compiler_flag(${flag} ${flagvar})
        elseif("${lang}" STREQUAL "CXX")
            check_cxx_compiler_flag(${flag} ${flagvar})
        endif()
    elseif("${type}" STREQUAL "LINKING")
        check_linker_flag(${lang} ${flag} ${flagvar})
    endif()

    # apply result

    if(${flagvar})
        if("${type}" STREQUAL "COMPILE")
            target_compile_options(${target} PRIVATE ${flag})
        elseif("${type}" STREQUAL "LINKING")
            target_link_options(${target} PRIVATE ${flag})
        endif()
    else()
        if(${required})
            message(FATAL_ERROR "Required ${type} flag ${flag} not found")
            return()
        endif()
    endif()
endfunction()
