#include <gtest/gtest.h>

#include <limits>

#include "baderror.h"
#include "complog/CompilationMessage.h"
#include "fixture.h"
#include "locators/locator.h"

TEST_F(PascalProgram, FullLine) {
    locators::Locator loc(file, file->Position(4, 24));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(100)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |    for i := 100 downto 1 do
                           ^
)%%%");
}

TEST_F(PascalProgram, MiddleOfLine) {
    locators::Locator loc(file, file->Position(4, 13));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(14)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |i := 100 do
        ^
)%%%");
}

TEST_F(PascalProgram, StartOfLine) {
    locators::Locator loc(file, file->Position(4, 4));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(17)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |    for i := 1
       ^
)%%%");
}

TEST_F(PascalProgram, EndOfLine) {
    locators::Locator loc(file, file->Position(4, 26));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(17)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |00 downto 1 do
               ^
)%%%");
}

TEST_F(PascalProgram, Overflow) {
    locators::Locator loc(file, file->Position(4, 24));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(std::numeric_limits<size_t>::max())),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |    for i := 100 downto 1 do
                           ^
)%%%");
}

TEST_F(PascalProgram, ZeroWidth) {
    locators::Locator loc(file, file->Position(4, 24));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(0)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |
   ^
)%%%");
}

TEST_F(PascalProgram, MinWidth) {
    locators::Locator loc(file, file->Position(4, 24));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(3)),
              R"%%%([Error] (BAD) Bad error
<string>:
5 |
   ^
)%%%");
}

TEST_F(PascalProgram, NoLocators) {
    locators::Locator loc(file, file->Position(4, 24));
    BadError err(loc);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions().WithWidth(100)), "[Error] (BAD) Bad error\n");
    locators::Locator loc2(file, 0);
    DoubleBadError err2(loc, loc2);
    EXPECT_EQ(err2.ToString(complog::CompilationMessage::FormatOptions().WithWidth(100)),
              "[Error] (DBLBAD) Very bad\n");
}

TEST_F(PascalProgram, TwoLocators) {
    locators::Locator loc(file, 0);
    locators::Locator loc2(file, file->Position(4, 24));
    DoubleBadError err(loc, loc2);
    EXPECT_EQ(err.ToString(complog::CompilationMessage::FormatOptions::All(100)),
              R"%%%([Error] (DBLBAD) Very bad
<string>:
1 |program test;
   ^
<string>:
5 |    for i := 100 downto 1 do
                           ^
)%%%");
}
