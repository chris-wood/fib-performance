cmake_minimum_required(VERSION 2.8)

project (fib_perf)
set(CMAKE_BUILD_TYPE Release)

#set(GCC_COVERAGE_COMPILE_FLAGS "-ftest-coverage -fprofile-arcs -pg")
#set(GCC_COVERAGE_LINK_FLAGS    "-pg")
#set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

include_directories(
        /usr/local/include/
        /Users/cwood/PARC/side/Improvements/src/Athena/
        /Users/cwood/PARC/side/Improvements/src/Libccnx-common
        /Users/cwood/PARC/side/Improvements/usr/include/
    )

set(fibtest_SOURCES
    src/fib_perf.c
    src/fib.c
    src/map.c
    src/siphash24.c
    src/random.c
    )

link_directories(
        /Users/cwood/PARC/side/Improvements/usr/lib
        /usr/local/lib
    )

add_executable(fib_perf ${fibtest_SOURCES})
set_target_properties(fib_perf PROPERTIES COMPILE_FLAGS "-Wall -pg")

target_link_libraries(fib_perf longbow longbow-ansiterm parc ccnx_common ccnx_api_portal ccnx_api_notify ccnx_api_control ccnx_transport_rta athena)

install(TARGETS fib_perf DESTINATION bin)
