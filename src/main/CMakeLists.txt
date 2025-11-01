target_sources(dinterp PRIVATE main.cpp syntaxExplorer.cpp)
target_link_libraries(dinterp PRIVATE common_features lexer syntaxer)
