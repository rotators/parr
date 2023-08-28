cmake_minimum_required( VERSION 3.18.4 FATAL_ERROR )

include( AddTestTarget )

enable_testing()
add_test_target( ${PRS_BIN_SSL} "--file=@filename@ --tokens --trace --tree" "ssl" ADD_GLOB "generic/*.t" )
