add_library(lexer STATIC lexer.cpp)
target_include_directories(lexer PUBLIC include)
target_link_libraries(lexer PRIVATE common_features)
target_link_libraries(lexer PUBLIC complog locators bigint)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:lexer>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/lexer.h
)

if (Testing)
    add_subdirectory(tests)
endif()
