set(PARR_LIBS_DIR "${CMAKE_CURRENT_LISTS_DIR}/libs")
function(parr_silence_includes target)
    get_target_property(include_dir ${target} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${target} SYSTEM INTERFACE ${include_dir})
endfunction()

#
# cxxopts
#

message(STATUS "Configure cxxopts library...")
set(CXXOPTS_ENABLE_WARNINGS TRUE)
add_subdirectory(cxxopts EXCLUDE_FROM_ALL)

#
# fmt
#

#message(STATUS "Configure fmt library...")
#set(PARR_INCLUDE_FMT "${PARSSL_LIBS_DIR}/fmt/include")
#add_subdirectory(fmt EXCLUDE_FROM_ALL)

#
# pegtl
#

message(STATUS "Configure PEGTL library...")
set(PEGTL_USE_FILESYSTEM FALSE)
add_subdirectory(pegtl EXCLUDE_FROM_ALL)
parr_silence_includes(pegtl)

file(REMOVE cmake_install.cmake)

message(STATUS "Configuring libraries complete")