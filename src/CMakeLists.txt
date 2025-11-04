cmake_minimum_required(VERSION 3.28)

project(dinterpreter VERSION 0.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE set to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(common_features INTERFACE)
target_compile_features(common_features INTERFACE cxx_std_20)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_options(common_features INTERFACE -Werror -Wall -Wextra)
    target_compile_definitions(common_features INTERFACE DINTERP_DEBUG)
endif()

option(CompileTests "Compile tests (requires GTest)" OFF)

if(CompileTests)
    find_package(GTest REQUIRED)
    enable_testing()
    add_library(test_features INTERFACE)
    target_compile_features(test_features INTERFACE cxx_std_20)
    target_link_libraries(test_features INTERFACE pthread GTest::gtest)
endif()

add_executable(dinterp)

add_subdirectory(bigint)
add_subdirectory(compilation_log)
add_subdirectory(locators)
add_subdirectory(lexer)
add_subdirectory(syntax)
add_subdirectory(main)
