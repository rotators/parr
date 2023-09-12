cmake_minimum_required( VERSION 3.18.4 FATAL_ERROR )

set( CTestRunner.FunctionName prs_test )
include( CTestRunner OPTIONAL RESULT_VARIABLE prs_ctest_runner )

if( NOT prs_ctest_runner )
    return()
endif()

enable_testing()
prs_test( ${PRS_BIN_PROCESSOR} "--file=@filename@ --tokens --trace --tree" "ssl" ADD_GLOB "generic/*.t" )
prs_test( ${PRS_BIN_SSL}       "--file=@filename@ --tokens --trace --tree" "ssl" ADD_GLOB "generic/*.t" )
