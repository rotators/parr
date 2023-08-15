cmake_minimum_required( VERSION 3.18.4 FATAL_ERROR )
include_guard()
#
## Directory structure
#
# Test/
#   my-program/          <- name of the target
#     my-test.txt        <- test file 
#     my-test.WILL_FAIL  <- (optional) test configuration file
#   CMakeLists.txt       <- add_test_target(my-program "-f @filename@" "txt")
#
## Command line modification
#
# .COMMAND_LINE
#                       if exists, .COMMAND_LINE_BEFORE and .COMMAND_LINE_AFTER are ignored
#
# .COMMAND_LINE_BEFORE
#                       ignored if .COMMAND_LINE exists
#
# .COMMAND_LINE_AFTER
#                       ignored if .COMMAND_LINE exists
#
## Test properties
## Allows setting selected CMake properties to test
## https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
#
# .DISABLED             https://cmake.org/cmake/help/latest/prop_test/DISABLED.html
# .WILL_FAIL            https://cmake.org/cmake/help/latest/prop_test/WILL_FAIL.html
#
function( add_test_target target command_line extension )
    set( debug FALSE )

    # check if enable_testing() was used
    if( NOT CMAKE_TESTING_ENABLED )
        if(debug)
            message( STATUS "Configuring tests skipped" )
        endif()
        return()
    endif()

    # search for test files
    file( GLOB_RECURSE found_tests LIST_DIRECTORIES false FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "${target}/*.${extension}" )
    list( SORT found_tests )

    # search for test configuration files
    foreach( tmp IN ITEMS COMMAND_LINE COMMAND_LINE_BEFORE COMMAND_LINE_AFTER WILL_FAIL DISABLED )
        file( GLOB_RECURSE tmp LIST_DIRECTORIES false FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "${target}/*.${tmp}" )
        unset( tmp )
    endforeach()

    foreach( file IN LISTS found_tests )

        # command line must be reset for each test
        set( test_command_line "${command_line}" )
        unset( test_command_line_before )
        unset( test_command_line_after )

        get_filename_component( name_dir "${file}" DIRECTORY )
        get_filename_component( name_wle "${file}" NAME_WLE )
        set( name "${name_dir}/${name_wle}" )
        set( filename "${CMAKE_CURRENT_LIST_DIR}/${name}" )

        list( APPEND used_files "${filename}.${extension}" )

        if( debug )
            message( STATUS "Add test... ${name}" )
            message( STATUS "- file: ${file}" )
            message( STATUS "- filename: ${filename}" )
        endif()

        if( EXISTS "${filename}.COMMAND_LINE" )
            file( READ "${filename}.COMMAND_LINE" test_command_line )

            list( APPEND used_files "${filename}.COMMAND_LINE" )
            if( debug )
                message( STATUS "- command line: ${test_command_line}" )
            endif()
        else()
            if( EXISTS "${filename}.COMMAND_LINE_BEFORE" )
                file( READ "${filename}.COMMAND_LINE_BEFORE" test_command_line_before )

                list( APPEND used_files "${filename}.COMMAND_LINE_BEFORE" )
                if( debug )
                    message( STATUS "- command line (before): ${test_command_line_before}" )
                endif()
            endif()

            if( EXISTS "${filename}.COMMAND_LINE_AFTER" )
                file( READ "${filename}.COMMAND_LINE" test_command_line_after )

                list( APPEND used_files "${filename}.COMMAND_LINE_AFTER" )
                if( debug )
                    message( STATUS "- command line (after):  ${test_command_line_after}" )
                endif()
            endif()
        endif()

        # replace command line placeholders with real values
        foreach( var IN ITEMS test_command_line test_command_line_before test_command_line_after )
            string( REPLACE "@filename@" "${filename}.${extension}" ${var} "${${var}}" )
        endforeach()

        add_test( NAME "${name}"
            COMMAND           "$<TARGET_FILE:${target}>" ${test_command_line_before} ${test_command_line} ${test_command_line_after}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        foreach( property IN ITEMS WILL_FAIL DISABLED )
            if( EXISTS "${filename}.${property}" )
                set_property( TEST "${name}" PROPERTY ${property} TRUE )

                list( APPEND used_files "${filename}.${property}" )
                if( debug )
                    message( STATUS "- property: ${property}" )
                endif()
            endif()
        endforeach( property )
    endforeach( file )

    if( CMAKE_BUILD_TYPE )
        set( TEST_CONFIG "${CMAKE_BUILD_TYPE}" )
    else()
        set( TEST_CONFIG "Release" )
    endif()

    add_custom_target( ${target}.test
        DEPENDS           ${target}
        COMMAND           ${CMAKE_CTEST_COMMAND} --build-config ${TEST_CONFIG} --output-on-failure
        SOURCES           ${used_files}
    )

    source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${found_tests} )
    source_group( "CMake"     REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )

    macro( _check_test_property )
        message(FATAL_ERROR)
    endmacro()
endfunction()
