cmake_minimum_required(VERSION 2.8)

project (fib_perf)
set(CMAKE_BUILD_TYPE Release)

#set(GCC_COVERAGE_COMPILE_FLAGS "-ftest-coverage -fprofile-arcs -pg")
#set(GCC_COVERAGE_LINK_FLAGS    "-pg")
#set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

include_directories(
        /usr/local/include/
        /usr/local/ccnx/dependencies/build/include
        /Users/cwood/PARC/side/Improvements/usr/include/
    )

set(libfib_SOURCES
    src/fib.c
    src/map.c
    src/siphash24.c
    src/random.c
    )

link_directories(
        /Users/cwood/PARC/side/Improvements/usr/lib
        /usr/local/lib
    )

set(fibtest_SOURCES
    ${libfib_SOURCES}
    src/fib_perf.c)

set(FIBTEST_LIBRARIES
       ccnx_common
       ccnx_api_portal
       ccnx_api_notify
       ccnx_transport_rta
       ccnx_api_control
       ccnx_common
       parc
       longbow
       longbow-ansiterm
       check)

macro(AddTest testFile)
    add_executable(${ARGV0} src/test/${ARGV0}.c ${libfib_SOURCES})
    target_link_libraries(${ARGV0} ${FIBTEST_LIBRARIES})
    add_test(${ARGV0} ${ARGV0})
endmacro(AddTest)

add_executable(fib_perf ${fibtest_SOURCES})
set_target_properties(fib_perf PROPERTIES COMPILE_FLAGS "-Wall -pg")
target_link_libraries(fib_perf ${FIBTEST_LIBRARIES})
install(TARGETS fib_perf DESTINATION bin)

AddTest(test_map)
