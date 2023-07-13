include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

# parr_build_option( target type guard required flag var )
#   target ...... name of the the target
#   type ........ string describing which CMake functions should be used to test and add <flag> to <target>
#                 "compile" ... test: check_cxx_compiler_flag()  add: target_compile_options()
#                 "link" ...... test: check_linker flag()        add: target_link_options()
#   guard .....,. name of variable used to test if given flag should be tested
#                 if variable resolves to FALSE, function does nothing
#   required .... name of variable used to test if given flag is mandatory
#                 if flag is not found and variable resolves to TRUE, configuration stops with error message
#   flag, var ... see CMake documentation
#                 https://cmake.org/cmake/help/v3.18/module/CheckCXXCompilerFlag.html
#                 https://cmake.org/cmake/help/v3.18/module/CheckLinkerFlag.html

function(parr_build_option target type guard required flag var)
    if(NOT ${guard})
        return()
    endif()

    if("${type}" STREQUAL "compile")
        check_cxx_compiler_flag(${flag} ${var})
    elseif("${type}" STREQUAL "link")
        check_linker_flag(CXX ${flag} ${var})
    else()
        message(AUTHOR_WARNING "Unknown flag type: ${type}")
        message(AUTHOR_WARNING "Valid flag types: compile, link")
        return()
    endif()

    if(${var})
        if("${type}" STREQUAL "compile")
            target_compile_options(${target} PRIVATE ${flag})
        elseif("${type}" STREQUAL "link")
            target_link_options(${target} PRIVATE ${flag})
        endif()
    else()
        if(${required})
            message(FATAL_ERROR "Required ${type} flag ${flag} not found")
            return()
        endif()
    endif()
endfunction()
