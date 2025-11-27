#include <gtest/gtest.h>

#include "dinterp/locators/locator.h"
#include "fixture.h"
using namespace std;
using namespace dinterp::locators;

TEST_F(RealCodeFixture, LocatorFromFile) {
    Locator loc(file, 34);
    EXPECT_EQ(loc.FileName(), "<string>");
    EXPECT_EQ(loc.Position(), 34);
    EXPECT_EQ(loc.Line(), 1);
    EXPECT_EQ(loc.Column(), 14);
    EXPECT_EQ(loc.Pretty(), "<string>:2:14");
    auto context = loc.Context(10, 10);
    EXPECT_EQ(context.Text, "g namespace std;");
    EXPECT_EQ(context.PointerWithinText, 10);
}
