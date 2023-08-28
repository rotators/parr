cmake_minimum_required( VERSION 3.18.4 FATAL_ERROR )
include_guard()
#
## Directory structure
#
# Test/
#   my-program/          <- name of the target
#     my-test.txt        <- test file
#     my-test.WILL_FAIL  <- (optional) one of test configuration files
#   CMakeLists.txt       <- add_test_target( my-program "-f @filename@" "txt" )
#
## Command line modification
#
# <test_file>.COMMAND_LINE
#   if exists, <test_file>.COMMAND_LINE_BEFORE and <test_file>.COMMAND_LINE_AFTER are ignored
#
# <test_file>.COMMAND_LINE_BEFORE
#   ignored if <test_file>.COMMAND_LINE exists
#
# <test_file>.COMMAND_LINE_AFTER
#   ignored if <test_file>.COMMAND_LINE exists
#
## Test properties
## Allows setting selected CMake test properties
## https://cmake.org/cmake/help/v3.18/manual/cmake-properties.7.html#properties-on-tests
#
# <test_file>.DISABLED
#   see https://cmake.org/cmake/help/v3.18/prop_test/DISABLED.html
# <test_file>.WILL_FAIL
#   see https://cmake.org/cmake/help/v3.18/prop_test/WILL_FAIL.html
#

function( add_test_target target command_line extension )
    # check if target is valid
    if( NOT TARGET ${target} )
        message( AUTHOR_WARNING "Target does not exist\n${target}" )
        return()
    endif()

    get_property( target_type TARGET ${target} PROPERTY TYPE )
    if( NOT "${target_type}" STREQUAL "EXECUTABLE" )
        message( AUTHOR_WARNING "Target is not execuatable\n${target}" )
        return()
    endif()

    # check if enable_testing() was used
    if( CMAKE_TESTING_ENABLED )
        # TODO: DEFER CALL (cmake v3.19)
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.help )
        message( STATUS "Configuring tests: ${target}" )
    else()
        message( STATUS "Configuring tests skipped: ${target}" )
        return()
    endif()

    # symbolic files created by add_test_target.help()
    set( help_targets    "${CMAKE_CURRENT_FUNCTION}.help.targets" )
    set( help_extensions "${CMAKE_CURRENT_FUNCTION}.help.extension" )

    cmake_parse_arguments( PARSE_ARGV 3 arg "" "" "ADD_GLOB" )

    # test configuration files extensions
    #  edit - require own logic
    #  prop - sets cmake test property with same name to TRUE
    set( test_config_edit COMMAND_LINE;COMMAND_LINE_BEFORE;COMMAND_LINE_AFTER )
    set( test_config_prop WILL_FAIL;DISABLED )

    # search for test files
    list( PREPEND arg_ADD_GLOB "${target}/*.${extension}" )
    foreach( find_glob IN LISTS arg_ADD_GLOB )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "Find test files... ${find_glob}" )

        file( GLOB_RECURSE found_tmp LIST_DIRECTORIES false FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "${find_glob}" )
        if( found_tmp )
            list( APPEND found_tests "${found_tmp}" )
        endif()

        # search for test configuration files and add them to VerifyGlobs
        get_filename_component( find_glob "${find_glob}" DIRECTORY )
        foreach( tmp IN LISTS test_config_edit test_config_prop )  #ITEMS COMMAND_LINE COMMAND_LINE_BEFORE COMMAND_LINE_AFTER WILL_FAIL DISABLED )
            cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug  "- Find property... ${find_glob}/*.${tmp}" )

            file( GLOB_RECURSE tmp LIST_DIRECTORIES false FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "${find_glob}/*.${tmp}" )
            unset( tmp )
        endforeach()
    endforeach()
    list( SORT found_tests )

    foreach( test_file IN LISTS found_tests )
        # reset variables holding test-specific data
        unset( test_used_files )

        set( test_command_line      "${command_line}" )
        set( test_command_line_edit FALSE )
        unset( test_command_line_before )
        unset( test_command_line_after )

        get_filename_component( test_file_dir "${test_file}" DIRECTORY )
        get_filename_component( test_file_wle "${test_file}" NAME_WLE )
        get_filename_component( test_file_ext "${test_file}" LAST_EXT )
        string( REPLACE "." "" test_file_ext "${test_file_ext}" )
        set( test_path_ext   "${CMAKE_CURRENT_LIST_DIR}/${test_file}" )
        set( test_path_noext "${CMAKE_CURRENT_LIST_DIR}/${test_file_dir}/${test_file_wle}" )

        # test name is set as relative path without extension with target name as prefix
        # allows adding same test file to multiple targets
        set( test_name "${target}::${test_file_dir}/${test_file_wle}" )

        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "Add test... ${test_name}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_file:        ${test_file}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_file_dir     ${test_file_dir}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_file_wle     ${test_file_wle}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_file_ext     ${test_file_ext}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_path_ext     ${test_path_ext}" )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_path_noext   ${test_path_noext}" )

        list( APPEND test_used_files "${test_path_ext}" )

        if( EXISTS "${test_path_noext}.COMMAND_LINE" )
            set( test_command_line_edit TRUE )
            file( READ "${test_path_noext}.COMMAND_LINE" test_command_line )

            list( APPEND test_used_files  "${test_path_noext}.COMMAND_LINE" )
        else()
            if( EXISTS "${test_path_noext}.COMMAND_LINE_BEFORE" )
                set( test_command_line_edit TRUE )
                file( READ "${test_path_noext}.COMMAND_LINE_BEFORE" test_command_line_before )

                list( APPEND test_used_files "${test_path_noext}.COMMAND_LINE_BEFORE" )
            endif()

            if( EXISTS "${test_path_noext}.COMMAND_LINE_AFTER" )
                set( test_command_line_edit TRUE )
                file( READ "${test_path_noext}.COMMAND_LINE" test_command_line_after )

                list( APPEND test_used_files "${test_path_noext}.COMMAND_LINE_AFTER" )
            endif()
        endif()

        # replace command line placeholders with real values
        foreach( var IN ITEMS test_command_line test_command_line_before test_command_line_after )
            string( REPLACE "@filename@" "${test_path_ext}" ${var} "${${var}}" )
        endforeach()
        string( STRIP "${test_command_line_before} ${test_command_line} ${test_command_line_after}" test_command_line_full )

        if( test_command_line_edit )
            cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- command line      ${test_command_line_full}" )
        endif()

        # [cmakepp] parses the command line string into parts (handling strings and semicolons)
        # https://github.com/toeb/cmakepp/blob/master/cmake/core/parse_command_line.cmake
        string( ASCII 31 tmp )
        string( REPLACE "\;" "${tmp}" tmp "${test_command_line_full}" )
        string( REGEX MATCHALL "((\\\"[^\\\"]*\\\")|[^ ]+)" tmp "${tmp}")
        string( REGEX REPLACE "(^\\\")|(\\\"$)" "" tmp "${tmp}")
        string( REGEX REPLACE "(;\\\")|(\\\";)" ";" tmp "${tmp}")
        string( REPLACE "\\" "/" test_command_line_full "${tmp}")

        add_test( NAME "${test_name}"
            COMMAND  "$<TARGET_FILE:${target}>" ${test_command_line_full}
        )

        # all created tests labels are using one of following formats
        #  <function_name>::<target>  primary label
        #  <function_name>:<name>     secondary label
        # primary labels allows to extract supported targets names via ctest
        #                set for all generated tests
        # secondary label allows
        #                set only if current test is enabled
        set_property( TEST "${test_name}" PROPERTY LABELS "${CMAKE_CURRENT_FUNCTION}::${target}" )

        foreach( property IN LISTS test_config_prop )
            if( EXISTS "${test_path_noext}.${property}" )
                cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- property          ${property} = TRUE" )
                set_property( TEST "${test_name}" PROPERTY ${property} TRUE )

                list( APPEND test_used_files   "${test_path_noext}.${property}" )
            endif()
        endforeach( property )

        list( APPEND target_used_files "${test_used_files}" )
        list( SORT test_used_files )
        foreach( file IN LISTS test_used_files )
            cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- test_used_files   ${file}" )
        endforeach()

        # disabled tests do not have secondary labels set, and cannot generate extra targets 
        get_test_property( "${test_name}" DISABLED disabled )
        if( disabled )
            continue()
        endif()

        # add extra labels:  must-fail, must-pass, extension:<?>
        # add extra targets: only.extension.<?> (if not exists)

        get_test_property( "${test_name}" WILL_FAIL status )
        if( status )
            set( status "fail")
        else()
            set( status "pass")
        endif()

        set_property( TEST "${test_name}" PROPERTY LABELS "${CMAKE_CURRENT_FUNCTION}:must-${status}" APPEND )
        set_property( TEST "${test_name}" PROPERTY LABELS "${CMAKE_CURRENT_FUNCTION}:extension:${test_file_ext}" APPEND )

        if( test_command_line_edit )
            set_property( TEST "${test_name}" PROPERTY LABELS "${CMAKE_CURRENT_FUNCTION}:command-line" APPEND )
        endif()

        if( NOT TARGET ${CMAKE_CURRENT_FUNCTION}.only.extension.${test_file_ext} )
            add_custom_target( ${CMAKE_CURRENT_FUNCTION}.only.extension.${test_file_ext}
                DEPENDS  ${target}
                COMMAND  ${CMAKE_CTEST_COMMAND} --output-on-failure --label-regex ^${CMAKE_CURRENT_FUNCTION}:extension:${test_file_ext}$
            )

            add_custom_command(
                OUTPUT  "${help_extensions}"
                COMMAND  "${CMAKE_COMMAND}" -E echo "  ${CMAKE_CURRENT_FUNCTION}.only.extension.${test_file_ext}"
                APPEND
            )
        endif()
    endforeach( test_file )

    cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "Add test target...  ${CMAKE_CURRENT_FUNCTION}.run.${target}" )
    list( SORT target_used_files )
    foreach( file IN LISTS target_used_files )
        cmake_language( CALL ${CMAKE_CURRENT_FUNCTION}.debug "- target_used_files ${file}" )
    endforeach()

    if( CMAKE_BUILD_TYPE )
        set( build_type "${CMAKE_BUILD_TYPE}" )
    else()
        set( build_type "Release" )
    endif()

    add_custom_target( ${CMAKE_CURRENT_FUNCTION}.run.${target}
        SOURCES  ${target_used_files}
        DEPENDS  ${target}
        COMMAND  "${CMAKE_CTEST_COMMAND}" --build-config ${build_type} --output-on-failure --label-regex ^${CMAKE_CURRENT_FUNCTION}::${target}$
    )

    add_custom_command(
        OUTPUT  "${help_targets}"
        COMMAND  "${CMAKE_COMMAND}" -E echo "  ${CMAKE_CURRENT_FUNCTION}.run.${target}"
        APPEND
    )

    source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${found_tests} )
    source_group( "CMake"     REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
endfunction()

function( add_test_target.debug message )
    set( debug 1 )

    if( debug )
        message( STATUS "${message}" )
    endif()
endfunction()

#
# Overengineered help screen
#

function( add_test_target.help )
    if( TARGET ${CMAKE_CURRENT_FUNCTION} )
        return()
    endif()

    message( STATUS "Configuring tests runner" )
    string( REGEX REPLACE "\\..+" "" main_function "${CMAKE_CURRENT_FUNCTION}" )
    string( REPLACE "${PROJECT_SOURCE_DIR}" "" current_file "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" )
    string( REGEX REPLACE "^[\\/]" "" current_file "${current_file}" )

    # NOTE: if changed, add_test_target() must be updated
    set( help_generic    "${CMAKE_CURRENT_FUNCTION}.generic" )
    set( help_targets    "${CMAKE_CURRENT_FUNCTION}.targets" )
    set( help_extensions "${CMAKE_CURRENT_FUNCTION}.extension" )

    if( CMAKE_BUILD_TYPE )
        set( build_type "${CMAKE_BUILD_TYPE}" )
    else()
        set( build_type "Release" )
    endif()

    # generic targets which run ctest with partial primary label
    set( help.run.all   "Run all tests added by ${main_function}() function" )
    add_custom_target( ${main_function}.run.all
        COMMAND  "${CMAKE_CTEST_COMMAND}" --build-config ${build_type} --output-on-failure --label-regex ^${main_function}::
    )

    # generic targets which run ctest with full secondary label
    set( help.only.command-line "Run all enabled tests with change command line arguments" )
    set( help.only.must-pass    "Run all enabled tests without WILL_FAIL property" )
    set( help.only.must-fail    "Run all enabled tests with WILL_FAIL property" )
    foreach( suffix IN ITEMS command-line must-pass must-fail )
        add_custom_target( ${main_function}.only.${suffix}
            COMMAND  "${CMAKE_CTEST_COMMAND}" --build-config ${build_type} --output-on-failure --label-regex ^${main_function}:${suffix}$
        )
    endforeach()

    # generic targets which run in script mode
    set( help.show.targets.gha "Print all targets list in format acceptable by GitHub Actions matrix" )
    set( help.show.labels      "Print all tests labels created by ${main_function}()" )
    foreach( suffix IN ITEMS show.targets.gha show.labels )
        set( call "${main_function}.script.${suffix}" )
        add_custom_target( ${main_function}.${suffix}
            COMMAND  "${CMAKE_COMMAND}" -DCALL="${call}" -P "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
        )
    endforeach()

    get_directory_property( targets BUILDSYSTEM_TARGETS )
    foreach( target IN LISTS targets )
        if( NOT "${target}" MATCHES "^${main_function}" )
            list( REMOVE_ITEM targets "${target}" )
            continue()
        endif()
        string( LENGTH "${target}" target_len )
        if( NOT targets_maxlen OR target_len GREATER targets_maxlen )
            set( targets_maxlen ${target_len} )
        endif()
    endforeach()

    # main target
    add_custom_target( ${CMAKE_CURRENT_FUNCTION}
        DEPENDS "${help_generic}" "${help_targets}" "${help_extensions}"
    )

    add_custom_command(
        OUTPUT   "${help_generic}"
        COMMENT  ""
        COMMAND  "${CMAKE_COMMAND}" -E echo "Generic targets created by ${current_file}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT   "${help_targets}"
        COMMENT  ""
        COMMAND  "${CMAKE_COMMAND}" -E echo ""
        COMMAND  "${CMAKE_COMMAND}" -E echo "Project targets created by ${current_file}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT   "${help_extensions}"
        COMMENT  ""
        COMMAND  "${CMAKE_COMMAND}" -E echo ""
        COMMAND  "${CMAKE_COMMAND}" -E echo "Extension targets created by ${current_file}"
        VERBATIM
    )

    set_property( SOURCE "${help_generic}" "${help_targets}" "${help_extensions}" PROPERTY SYMBOLIC TRUE )

    set( help.check "Check tests runner integrity; all targets above are executed one by one (internal use)")
    set( check_self ${main_function}.check.self )
    add_custom_target( ${main_function}.check DEPENDS "${check_self}" )
    add_custom_command( OUTPUT "${check_self}" COMMENT "" VERBATIM )
    set_property( SOURCE "${check_self}" PROPERTY SYMBOLIC TRUE )
    list( APPEND targets ${main_function}.check )

    foreach( target IN LISTS targets )
        string( LENGTH "${target}" target_len )
        math( EXPR target_spaces "${targets_maxlen} - ${target_len}" )
        string( REPEAT " " ${target_spaces} target_spaces )
        string( REPLACE "${main_function}" "help" target_help "${target}" )
        if( ${target_help} )
            set( target_help "  ${target}${target_spaces}  ${${target_help}}" )
        else()
            message( AUTHOR_WARNING "Target is missing description\n${target}" )
            set( target_help "  ${target}" )
        endif()

        add_custom_command(
            OUTPUT   "${help_generic}"
            COMMAND  "${CMAKE_COMMAND}" -E echo "${target_help}"
            APPEND
        )

        if( "${target}" STREQUAL "${main_function}.check" )
            continue()
        endif()

        add_custom_command(
            OUTPUT "${check_self}"
            COMMAND "${CMAKE_COMMAND}" -E echo "Checking target: ${target}"
            COMMAND "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}" --config ${build_type} --target "${target}"
            APPEND
        )
    endforeach()
endfunction()

#
# Script mode
#

if( NOT CMAKE_ARGC )
    return()
endif()

foreach( name IN ITEMS CALL )
    if( NOT ${name} )
        message( FATAL_ERROR "Required internal variable ${name} not set" )
        return()
    #else()
    #    message( STATUS "${name} = ${${name}}" )
    endif()
endforeach()

function( add_test_target.LABELS var )
    string( REGEX REPLACE "\\..+" "" main_function "${CMAKE_CURRENT_FUNCTION}" )
    execute_process(
        COMMAND         ${CMAKE_CTEST_COMMAND} --test-dir "${CMAKE_CURRENT_BINARY_DIR}" --print-labels
        OUTPUT_VARIABLE output
    )

    string( REPLACE "\n" ";" output "${output}" )
    foreach( line IN LISTS output )
        if( "${line}" MATCHES "^[\t ]+(${main_function}:.+)" )
            list( APPEND labels "${CMAKE_MATCH_1}" )
        endif()
    endforeach()

    set( ${var} "${labels}" PARENT_SCOPE )
endfunction()

function( add_test_target.script.show.labels )
    string( REGEX REPLACE "\\..+" "" main_function "${CMAKE_CURRENT_FUNCTION}" )
    cmake_language( CALL ${main_function}.LABELS labels )

    message( "All labels created by ${main_function}()")
    foreach( label IN LISTS labels )
        message( "  ${label}" )
    endforeach()
endfunction()

function( add_test_target.TARGETS var )
    string( REGEX REPLACE "\\..+" "" main_function "${CMAKE_CURRENT_FUNCTION}" )
    cmake_language( CALL ${main_function}.LABELS labels )

    foreach( label IN LISTS labels )
        if( "${label}" MATCHES "^${main_function}::(.+)" )
            list( APPEND targets "${CMAKE_MATCH_1}" )
        endif()
    endforeach()

    set( ${var} "${targets}" PARENT_SCOPE )
endfunction()

function( add_test_target.script.show.targets.gha )
    string( REGEX REPLACE "\\..+" "" main_function "${CMAKE_CURRENT_FUNCTION}" )
    cmake_language( CALL ${main_function}.TARGETS targets_tmp )

    message( "All targets handled by ${main_function}()" )
    foreach( target IN LISTS targets_tmp )
        list( APPEND targets "{'name':'${target}','cmake-target':'${main_function}.run.${target}'}" )
    endforeach()

    string( REPLACE ";" "," targets "${targets}" )
    string( REPLACE "'" "\\\"" targets "${targets}" ) 
    message( "[${targets}]" )
endfunction()

cmake_language( CALL "${CALL}")
