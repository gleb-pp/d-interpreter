cmake_minimum_required(VERSION 3.28)
project(usr VERSION 0.0.0 LANGUAGES CXX)

find_package(DInterp 1.0.0 EXACT REQUIRED)

add_executable(greeter main.cpp)
target_compile_features(greeter PRIVATE cxx_std_14)  # test if it inherits c++20 from the library
target_compile_options(greeter PRIVATE -O3 -Wall -Werror -Wextra)
target_link_libraries(greeter PRIVATE DInterp::dinterptools)
add_test(NAME testrun COMMAND greeter)
