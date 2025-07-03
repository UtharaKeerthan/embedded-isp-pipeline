cmake_minimum_required(VERSION 3.16)
project(embedded_isp_pipeline CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Embedded-safe compile flags (MISRA-aligned)
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wstack-usage=4096
    -fno-exceptions
    -fno-rtti
    -Werror=return-type
)

if(SIL_MODE)
    add_definitions(-DSIL_MODE=1)
    message(STATUS "Building in SIL (Software-in-the-Loop) mode")
endif()

# Collect all source files
file(GLOB_RECURSE SOURCES
    src/core/*.cpp
    src/color/*.cpp
    src/filter/*.cpp
    src/edge/*.cpp
    src/geometry/*.cpp
    src/analysis/*.cpp
)

add_executable(isp_pipeline ${SOURCES})
target_include_directories(isp_pipeline PRIVATE include)

# Unit tests (requires GoogleTest)
find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES tests/unit/*.cpp tests/integration/*.cpp)
    add_executable(isp_tests ${TEST_SOURCES} ${SOURCES})
    target_include_directories(isp_tests PRIVATE include)
    target_link_libraries(isp_tests GTest::GTest GTest::Main)
    add_test(NAME isp_unit_tests COMMAND isp_tests)
endif()
