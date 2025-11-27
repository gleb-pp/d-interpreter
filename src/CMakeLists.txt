cmake_minimum_required(VERSION 3.28)

set(version_maj 1)
set(version ${version_maj}.0.0)
project(dinterpreter VERSION ${version} LANGUAGES CXX)

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

include(GNUInstallDirs)

add_executable(dinterp)
add_library(dinterptools)
set_property(TARGET dinterptools PROPERTY VERSION ${version})
set_property(TARGET dinterptools PROPERTY SOVERSION ${version_maj})

add_subdirectory(bigint)
add_subdirectory(complog)
add_subdirectory(locators)
add_subdirectory(lexer)
add_subdirectory(syntax)
add_subdirectory(runtime)
add_subdirectory(semantic)
add_subdirectory(interp)
add_subdirectory(main)

include(CMakePackageConfigHelpers)
install(TARGETS dinterptools EXPORT DInterpExport FILE_SET HEADERS)
install(EXPORT DInterpExport FILE DInterpTargets.cmake NAMESPACE DInterp:: DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/dinterp)
configure_package_config_file(
    dinterpreterConfig.cmake.in
    dinterpreterConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dinterp
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/dinterpreterConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dinterpreterConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/dinterpreterConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dinterp
)
