cmake_minimum_required(VERSION 2.8)

project (fib_perf)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_STANDARD 99)
set(CCNX_INCLUDE $ENV{CCNX_HOME}/include)
set(CCNX_LIB $ENV{CCNX_HOME}/lib)

MESSAGE("${CCNX_INCLUDE}")
MESSAGE("${CCNX_LIB}")

include_directories(
        /usr/local/include/
        /usr/local/ccnx/dependencies/build/include/
        ${CCNX_INCLUDE}
    )

set(libfib_SOURCES
    src/fib.c
    src/fib_cisco.c
    src/fib_naive.c
    src/fib_caesar.c
    src/map.c
    src/name.c
    src/hasher.c
    src/patricia.c
    src/siphash24.c
    src/random.c
    src/bloom.c
    src/prefix_bloom.c
    )

link_directories(
        ${CCNX_LIB}
        /usr/local/lib
    )

set(fibtest_SOURCES
    ${libfib_SOURCES}
    src/fib-perf.c)

set(FIBTEST_LIBRARIES
       ccnx_common
       ccnx_api_portal
       ccnx_api_notify
       ccnx_transport_rta
       ccnx_api_control
       ccnx_common
       parc
       longbow
       longbow-ansiterm)

macro(AddTest testFile)
    add_executable(${ARGV0} src/test/${ARGV0}.c ${libfib_SOURCES})
    target_link_libraries(${ARGV0} ${FIBTEST_LIBRARIES})
    add_test(${ARGV0} ${ARGV0})
    install(TARGETS ${ARGV0} DESTINATION bin)
endmacro(AddTest)

SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")
add_executable(fib_perf ${fibtest_SOURCES})
set_target_properties(fib_perf PROPERTIES COMPILE_FLAGS "-Wall -pg")
target_link_libraries(fib_perf ${FIBTEST_LIBRARIES})
install(TARGETS fib_perf DESTINATION bin)

enable_testing()

AddTest(test_map)
AddTest(test_patricia)
AddTest(test_bloom)
AddTest(test_name)
AddTest(test_prefix_bloom)
