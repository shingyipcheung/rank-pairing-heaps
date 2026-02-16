cmake_minimum_required(VERSION 3.14)
project(astarheap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(astarheap
    example/main.cpp
)

target_include_directories(astarheap PRIVATE example)

# Copy map data file next to the executable so the default path works
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/map)
configure_file(
    ${CMAKE_SOURCE_DIR}/example/102_000_00033.bin
    ${CMAKE_BINARY_DIR}/map/102_000_00033.bin
    COPYONLY
)

# ---- Testing ----
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_rp_heap test/test_rp_heap.cpp)
target_include_directories(test_rp_heap PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_rp_heap GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_rp_heap)

# ---- Benchmarking ----
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(bench_rp_heap bench/bench_rp_heap.cpp)
target_include_directories(bench_rp_heap PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(bench_rp_heap benchmark::benchmark_main)
