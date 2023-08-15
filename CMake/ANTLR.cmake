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

    get_filename_component(jar_dir "${jar}" DIRECTORY)

    set(lexer  "${grammar}Lexer" )
    set(parser "${grammar}Parser" )

    set(g4lexer  "${PROJECT_SOURCE_DIR}/grammar/${lexer}.g4")
    set(g4parser "${PROJECT_SOURCE_DIR}/grammar/${parser}.g4")

    set(antlr antlr4)

    # fix variables used in add_custom_command() when running on cygwin
    # java won't understand cygwin-style paths, convert to pseudo-windows (c:/dir/file) manually
    foreach(tmp IN ITEMS jar jar_dir g4lexer g4parser)
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
            "${jar_dir}/${lib}/${lexer}.cpp"
            "${jar_dir}/${lib}/${lexer}.h"

            "${jar_dir}/${lib}/${parser}.cpp"
            "${jar_dir}/${lib}/${parser}.h"

            # "${jar_dir}/${lib}/${parser}BaseListener.cpp"
            # "${jar_dir}/${lib}/${parser}BaseListener.h"
            # "${jar_dir}/${lib}/${parser}Listener.cpp"
            # "${jar_dir}/${lib}/${parser}Listener.h"

            # "${jar_dir}/${lib}/${parser}BaseVisitor.cpp"
            # "${jar_dir}/${lib}/${parser}BaseVisitor.h"
            # "${jar_dir}/${lib}/${parser}Visitor.cpp"
            # "${jar_dir}/${lib}/${parser}Visitor.h"
    )

    #
    foreach(extension IN ITEMS cpp hpp)
        string(REGEX REPLACE "\.cmake$" ".${extension}" tmp "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
        if(EXISTS "${tmp}.in")
            configure_file("${tmp}.in" "${jar_dir}/${lib}/${namespace}.${lib}.${extension}" @ONLY NEWLINE_STYLE UNIX)
            target_sources(${namespace}.${lib} PRIVATE "${jar_dir}/${lib}/${namespace}.${lib}.${extension}" )
        endif()
    endforeach()

    set_target_properties(${namespace}.${lib} PROPERTIES ANTLR_LIBRARY TRUE)
    target_include_directories(${namespace}.${lib} SYSTEM PUBLIC "${jar_dir}/${lib}/")
    target_include_directories(${namespace}.${lib} SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Libs/antlr/runtime/Cpp/runtime/src/")
    target_compile_definitions(${namespace}.${lib} PUBLIC "ANTLR4_USE_THREAD_LOCAL_CACHE=1")
    target_link_libraries(${namespace}.${lib} PUBLIC antlr4_static)
    get_property(sources TARGET ${namespace}.${lib} PROPERTY SOURCES)

    # run antlr on demand

    list( APPEND command_line_cpp  -Dlanguage=Cpp )
    list( APPEND command_line_cpp  -package ${namespace}::antlr )
    list( APPEND command_line_cpp  -o "\"${jar_dir_path}/${lib}\"" )

    list( APPEND command_line_java -Dlanguage=Java )
    list( APPEND command_line_java -package ${namespace}.${lib} )
    list( APPEND command_line_java -o "\"${jar_dir_path}/${lib}/java\"" )

    foreach( tmp IN ITEMS command_line_cpp command_line_java )
        list( PREPEND ${tmp} -jar "\"${jar_path}\"" )
        list( APPEND  ${tmp} -no-listener )
        list( APPEND  ${tmp} -no-visitor )
        list( APPEND  ${tmp} "\"${g4lexer_path}\"" )
        list( APPEND  ${tmp} "\"${g4parser_path}\"" )
    endforeach()

    add_custom_command(
        OUTPUT      ${sources}
        BYPRODUCTS  "${jar_dir}/${lib}/${lexer}.interp" "${jar_dir}/${lib}/${lexer}.tokens" "${jar_dir}/${lib}/${parser}.interp" "${jar_dir}/${lib}/${parser}.tokens"
        COMMAND     "${Java_JAVA_EXECUTABLE}" ${command_line_cpp}
        DEPENDS     "${jar}" "${g4lexer}" "${g4parser}"
    )

    add_custom_command(
        OUTPUT           "${jar_dir}/${lib}/java/${lexer}.java" "${jar_dir}/${lib}/java/${parser}.java"
        COMMAND          "${Java_JAVA_EXECUTABLE}" ${command_line_java}
        DEPENDS          ${sources}
    )
#[[
    add_custom_command(
        OUTPUT            "${jar_dir}/${lib}.jar"
        COMMAND           "${Java_JAVA_EXECUTABLE}" -jar "${jar_path}" -Dlanguage=Java -package ${namespace}.${lib} -o "${jar_dir_path}/${lib}/java" -no-listener -no-visitor "${g4lexer_path}" "${g4parser_path}"
        COMMAND           "${Java_JAVAC_EXECUTABLE}" -cp "${jar_path}" "${jar_dir_path}/${lib}/java/*.java"
        COMMAND           "${Java_JAR_EXECUTABLE}" -c -f "${jar_dir_path}/${lib}.jar" "${jar_dir_path}/${lib}/java/*.class"

        DEPENDS           "${jar}" "${g4lexer}" "${g4parser}"
        MAIN_DEPENDENCY   ${sources}
    )
]]
endfunction()