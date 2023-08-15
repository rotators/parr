set(PARR_LIBS_DIR "${CMAKE_CURRENT_LISTS_DIR}/libs")

function(parr_silence_includes target)
    get_target_property(include_dir ${target} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${target} SYSTEM INTERFACE ${include_dir})
endfunction()

#
# antlr
#

message(STATUS "Configuring antlr...")
set(ANTLR_BUILD_CPP_TESTS OFF)
set(ANTLR_BUILD_SHARED    OFF)
add_subdirectory(antlr/runtime/Cpp EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME}+antlr INTERFACE)
target_include_directories(${PROJECT_NAME}+antlr SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/antlr/runtime/Cpp/runtime/src)
target_link_libraries(${PROJECT_NAME}+antlr INTERFACE antlr4_static)

#
# pegtl
#

#message(STATUS "Configuring PEGTL library...")
#set(PEGTL_USE_FILESYSTEM FALSE)
#add_subdirectory(pegtl EXCLUDE_FROM_ALL)
#parr_silence_includes(pegtl)

#
# cxxopts
#

message(STATUS "Configuring cxxopts library...")
set(CXXOPTS_ENABLE_WARNINGS TRUE)
add_subdirectory(cxxopts EXCLUDE_FROM_ALL)

file(REMOVE cmake_install.cmake)

message(STATUS "Configuring libraries complete")