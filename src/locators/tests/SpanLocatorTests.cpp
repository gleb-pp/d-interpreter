#include <gtest/gtest.h>

#include "fixture.h"
#include "dinterp/locators/locator.h"
using namespace std;
using namespace locators;

TEST_F(RealCodeFixture, spanLocTest) {
    size_t start = file->Position(1, 1);
    size_t len = 10;
    SpanLocator loc(file, start, len);
    EXPECT_EQ(loc.End().Position(), start + len);
    EXPECT_EQ(loc.Excerpt(), "sing names");
}
