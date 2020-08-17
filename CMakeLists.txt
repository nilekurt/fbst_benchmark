cmake_minimum_required(VERSION 3.14)

project(fbst_benchmark VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

include(FetchContent)
include(lto.helper)
include(test.helper)
include(with.helper)
include(tidy.helper)

option(FBST_BENCHMARK_CLANG_TIDY "Build with clang-tidy" ON)
option(FBST_BENCHMARK_TESTS "Build and add tests" ON)
option(FBST_BENCHMARK_COVERAGE "Build with coverage support" OFF)
option(LTO "Build with Link-Time Optimization" OFF)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
    set(FBST_BENCHMARK_CLANG_TIDY OFF)
    # Disable DeathTests
    set(ENV{EXTRA_TEST_ARGS} --gtest_filter=-*DeathTest*)
endif()

WithMsg(tests ${FBST_BENCHMARK_TESTS})
WithMsg(clang-tidy ${FBST_BENCHMARK_CLANG_TIDY})
WithMsg(coverage ${FBST_BENCHMARK_COVERAGE})

if (FBST_BENCHMARK_CLANG_TIDY)
    include(clang-tidy)
endif()

set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --child-silent-after-fork=yes --leak-check=full --error-exitcode=5")

if (FBST_BENCHMARK_TESTS)
    include(CTest)
    enable_testing()
endif()

set(FBST_BENCHMARK_EXTRA_CFLAGS -O3 -ffast-math -fvisibility=hidden)

set_property(GLOBAL PROPERTY FBST_BENCHMARK_RELEASE_EXTRA_CFLAGS ${FBST_BENCHMARK_EXTRA_CFLAGS})

add_library(fbst_benchmark_copts_common INTERFACE)
target_compile_options(fbst_benchmark_copts_common INTERFACE
    -pedantic
    -W
    -Wall
    -Wextra
    -Wcast-align
    -Wcast-qual
    -Wmissing-declarations
    -Wwrite-strings
    -Wundef
    -Wswitch-enum
    -Wshadow
    -Werror
    -Wstrict-aliasing=2
    $<$<C_COMPILER_ID:Clang>:-Wshadow-all>
    $<$<CXX_COMPILER_ID:Clang>:-Wshadow-all>
    $<$<STREQUAL:$<CONFIG>,Debug>:-ggdb>
    $<$<STREQUAL:$<CONFIG>,Release>:${FBST_BENCHMARK_EXTRA_CFLAGS}>
    $<$<BOOL:${FBST_BENCHMARK_COVERAGE}>:-g -fprofile-arcs -ftest-coverage>)
target_link_libraries(fbst_benchmark_copts_common
    INTERFACE
    $<$<BOOL:${FBST_BENCHMARK_COVERAGE}>:
        $<IF:$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>,
            gcov, # GCC
            --coverage # Clang
        >
    >)


add_subdirectory(3rdparty)

add_subdirectory(fbst)
add_subdirectory(fbst_benchmark_main)
