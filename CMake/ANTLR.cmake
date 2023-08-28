include_guard()

function(project_antlr_download url jar)
    get_filename_component(jar_dir "${jar}" DIRECTORY)

    add_custom_command(
        OUTPUT      "${jar}"
        BYPRODUCTS  "${jar}.cmake"
        COMMAND     "${CMAKE_COMMAND}" -E echo "file(DOWNLOAD \"${url}\" \"${jar}\")" > "${jar}.cmake"
        COMMAND     "${CMAKE_COMMAND}" -P "${jar}.cmake"
        VERBATIM
    )
endfunction()

#
# replacement for stock antlr_target()
# all preparations are baked into build step instead of configuration step
#
function(project_antlr_library namespace lib grammar jar)
    #
    # find java
    #

    find_package(Java REQUIRED COMPONENTS Runtime )#OPTIONAL_COMPONENTS Development)

    #
    # prepare all variables
    #

    cmake_parse_arguments(PARSE_ARGV 4 arg "LISTENER VISITOR" "" "")

    get_filename_component(jar_dir "${jar}" DIRECTORY)

    set(lexer  "${grammar}Lexer" )
    set(parser "${grammar}Parser" )

    set(g4lexer  "${PROJECT_SOURCE_DIR}/grammar/${lexer}.g4")
    set(g4parser "${PROJECT_SOURCE_DIR}/grammar/${parser}.g4")

    set(listener "-no-listener")
    set(listener_cpp "//")

    set(visitor  "-no-visitor")
    set(visitor_cpp "//")

    if(arg_LISTENER)
        set(listener "-listener")
        unset(listener_cpp)
    endif()

    if(arg_VISITOR)
        set(visitor "-visitor")
        unset(visitor_cpp)
    endif()

    set(antlr antlr4)

    # check if both .g4 are present
    foreach(g4 IN ITEMS g4lexer g4parser)
        if(NOT EXISTS "${${g4}}")
            message(AUTHOR_WARNING "Grammar file does not exist: ${${g4}}")
            return()
        endif()
    endforeach()

    foreach(tmp IN ITEMS cpp java)
        set(dir_${tmp} "${jar_dir}/${lib}/${tmp}")
    endforeach()

    # fix variables used in add_custom_command() when running on cygwin
    # java won't understand cygwin-style paths, convert to pseudo-windows (c:/dir/file) manually
    foreach(tmp IN ITEMS jar jar_dir g4lexer g4parser dir_cpp dir_java)
        if(CYGWIN)
            string(REGEX REPLACE "/cygdrive/([a-z])/" "\\1:/" ${tmp}_path "${${tmp}}")
        else()
            set(${tmp}_path "${${tmp}}")
        endif()
    endforeach()

    # create cmake target

    message(STATUS "Creating ANTLR target: ${namespace}.${lib}")

    add_library(${namespace}.${lib} STATIC)
    target_sources(${namespace}.${lib}
        PRIVATE
            "${dir_cpp}/${lexer}.cpp"
            "${dir_cpp}/${lexer}.h"

            "${dir_cpp}/${parser}.cpp"
            "${dir_cpp}/${parser}.h"
    )
    list(APPEND sources_java "${dir_java}/${lexer}.java")
    list(APPEND sources_java "${dir_java}/${parser}.java")

    if( arg_LISTENER )
        target_sources(${namespace}.${lib}
            PRIVATE
                "${dir_cpp}/${parser}BaseListener.cpp"
                "${dir_cpp}/${parser}BaseListener.h"
                "${dir_cpp}/${parser}Listener.cpp"
                "${dir_cpp}/${parser}Listener.h"
        )
        list(APPEND sources_java "${dir_java}/BaseListener.java")
        list(APPEND sources_java "${dir_java}/Listener.java")
    endif()

    if(arg_VISITOR)
        target_sources(${namespace}.${lib}
            PRIVATE
                "${dir_cpp}/${parser}BaseVisitor.cpp"
                "${dir_cpp}/${parser}BaseVisitor.h"
                "${dir_cpp}/${parser}Visitor.cpp"
                "${dir_cpp}/${parser}Visitor.h"
        )
        list(APPEND sources_java "${dir_java}/BaseVisitor.java")
        list(APPEND sources_java "${dir_java}/Visitor.java")
    endif()

    #
    foreach(extension IN ITEMS cpp hpp)
        string(REGEX REPLACE "\.cmake$" ".${extension}" tmp "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
        if(EXISTS "${tmp}.in")
            configure_file("${tmp}.in" "${dir_cpp}/${namespace}.${lib}.${extension}" @ONLY NEWLINE_STYLE UNIX)
            target_sources(${namespace}.${lib} PRIVATE "${dir_cpp}/${namespace}.${lib}.${extension}" )
        endif()
    endforeach()

    set_target_properties(${namespace}.${lib} PROPERTIES ANTLR_LIBRARY TRUE)
    target_include_directories(${namespace}.${lib} SYSTEM PUBLIC "${dir_cpp}/")
    target_include_directories(${namespace}.${lib} SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Libs/antlr/runtime/Cpp/runtime/src/")
    target_compile_definitions(${namespace}.${lib} PUBLIC "ANTLR4_USE_THREAD_LOCAL_CACHE=1")
    target_link_libraries(${namespace}.${lib} PUBLIC antlr4_static)
    get_property(sources_cpp TARGET ${namespace}.${lib} PROPERTY SOURCES)

    # run antlr on demand

    list( APPEND command_line_cpp  -Dlanguage=Cpp )
    list( APPEND command_line_cpp  -package ${namespace}::antlr )

    list( APPEND command_line_java -Dlanguage=Java )
    #list( APPEND command_line_java -package ${namespace}.${lib} )

    foreach( tmp IN ITEMS cpp java )
        list( PREPEND command_line_${tmp} -Werror )
        list( PREPEND command_line_${tmp} -jar "\"${jar_path}\"" )
        list( APPEND  command_line_${tmp} ${listener} )
        list( APPEND  command_line_${tmp} ${visitor} )
        list( APPEND  command_line_${tmp} -o "\"${dir_${tmp}_path}\"" )
        list( APPEND  command_line_${tmp} "\"${g4lexer_path}\"" )
        list( APPEND  command_line_${tmp} "\"${g4parser_path}\"" )

        add_custom_command(
            OUTPUT      ${sources_${tmp}}
            BYPRODUCTS  "${dir_${tmp}}/${lexer}.interp" "${dir_${tmp}}/${lexer}.tokens" "${dir_${tmp}}/${parser}.interp" "${dir_${tmp}}/${parser}.tokens"
            COMMAND     "${Java_JAVA_EXECUTABLE}" ${command_line_${tmp}}
            DEPENDS     "${jar}" "${g4lexer}" "${g4parser}"
        )
    endforeach()

    if(NOT Java_JAVAC_EXECUTABLE)
        return()
    endif()

    add_custom_command(
        OUTPUT            "${jar_dir}/${lib}.jar"
        COMMAND           "${Java_JAVAC_EXECUTABLE}" -d "${dir_java_path}/class" -cp "${jar_path}" "${dir_java_path}/*.java"
        COMMAND           "${Java_JAR_EXECUTABLE}" -c -f "${jar_dir_path}/${lib}.jar" "${jar_dir_path}/class/*.class"
        DEPENDS           "${jar}" "${g4lexer}" "${g4parser}"
        MAIN_DEPENDENCY   "{g4lexer}"
    )
endfunction()
