add_library(runtime types.cpp values.cpp derror.cpp)
target_link_libraries(runtime PRIVATE common_features)
target_link_libraries(runtime PUBLIC syntaxer lexer complog locators)
target_include_directories(runtime PUBLIC include)
