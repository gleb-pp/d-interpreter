add_library(syntaxer syntax.cpp exceptions.cpp syntaxcontext.cpp tokenscanner.cpp astvisitor.cpp)
target_link_libraries(syntaxer PUBLIC lexer common_features)
target_include_directories(syntaxer PUBLIC include)

target_sources(dinterptools PRIVATE $<TARGET_OBJECTS:syntaxer>)
target_include_directories(dinterptools INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_sources(dinterptools PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/dinterp/asterrors.h
    include/dinterp/syntax.h
)

if (Testing)
    add_subdirectory(tests)
endif()
