cmake_minimum_required( VERSION 3.18.4 FATAL_ERROR )

enable_testing()

function( add_test_target target extension )
    message( STATUS "Target: ${target}" )
    file( GLOB_RECURSE found_tests LIST_DIRECTORIES false FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "${target}/*.${extension}" )
    list( SORT found_tests )

    foreach( file IN LISTS found_tests )
        get_filename_component( name_dir "${file}" DIRECTORY )
        get_filename_component( name_wle "${file}" NAME_WLE )
        set( name "${name_dir}/${name_wle}" )
        set( filename "${CMAKE_CURRENT_LIST_DIR}/${name}" )

        list( APPEND used_files "${filename}.${extension}" )

#        message( STATUS "Add test... ${name}" )
#        message( STATUS "- file: ${file}" )
#        message( STATUS "- filename: ${filename}" )

        # default command line, used for all tests
        set( command_line --file "${CMAKE_CURRENT_SOURCE_DIR}/${file}" --run=parse-error-trace )

        if( EXISTS "${filename}.COMMAND_LINE" )
            file( READ "${filename}.COMMAND_LINE" command_line )

            list( APPEND used_files "${filename}.COMMAND_LINE" )
#            message( STATUS "- command line: ${command_line}" )
        else()
            if( EXISTS "${filename}.COMMAND_LINE_BEFORE" )
                file( READ "${filename}.COMMAND_LINE_BEFORE" command_line_before )

                list( APPEND used_files "${filename}.COMMAND_LINE_AFTER" )
#                message( STATUS "- command line (before): ${command_line_after}" )
            endif()

            if( EXISTS "${filename}.COMMAND_LINE_AFTER" )
                file( READ "${filename}.COMMAND_LINE" command_line_after )

                list( APPEND used_files "${filename}.COMMAND_LINE_AFTER" )
#                message( STATUS "- command line (after):  ${command_line_after}" )
            endif()
        endif()

        add_test( NAME "${name}"
            COMMAND           "$<TARGET_FILE:${target}>" ${command_line_before} ${command_line} ${command_line_after}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        if( EXISTS "${filename}.WILL_FAIL" )
            set_property( TEST "${name}" PROPERTY WILL_FAIL TRUE )

            list( APPEND used_files "${filename}.WILL_FAIL" )
#            message( STATUS "- will fail" )
        endif()
    endforeach()

    if( CMAKE_BUILD_TYPE )
        set( TEST_CONFIG "${CMAKE_BUILD_TYPE}" )
    else()
        set( TEST_CONFIG "Release" )
    endif()

    add_custom_target( ${target}-test
        DEPENDS           ${target}
        COMMAND           ${CMAKE_CTEST_COMMAND} --build-config ${TEST_CONFIG} --output-on-failure
        SOURCES           ${used_files}
    )

    source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${found_tests} )
    source_group( "CMake"     REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
endfunction()

add_test_target( ${PARR_SSL} "ssl" )
