#include <gtest/gtest.h>

#include <stdexcept>

#include "dinterp/complog/CompilationMessage.h"
using namespace std;
using namespace dinterp;
using namespace complog;

TEST(SeverityTests, Ctor) {
    EXPECT_THROW(Severity(-12), runtime_error);
    EXPECT_THROW(Severity(-1), runtime_error);
    EXPECT_THROW(Severity(3), runtime_error);
    EXPECT_THROW(Severity(100), runtime_error);
    EXPECT_NO_THROW(Severity(0));
    EXPECT_NO_THROW(Severity(1));
    EXPECT_NO_THROW(Severity(2));
}

TEST(SeverityTests, Indices) {
    EXPECT_EQ(Severity::Error().Index(), 2);
    EXPECT_EQ(Severity::Warning().Index(), 1);
    EXPECT_EQ(Severity::Info().Index(), 0);
    EXPECT_EQ(Severity::Error(), Severity(2));
    EXPECT_EQ(Severity::Warning(), Severity(1));
    EXPECT_EQ(Severity::Info(), Severity(0));
}

TEST(SeverityTests, Comparisons) {
    EXPECT_EQ(Severity::Error(), Severity::Error());
    EXPECT_EQ(Severity::Warning(), Severity::Warning());
    EXPECT_EQ(Severity::Info(), Severity::Info());
    Severity err = Severity::Error(), warning = Severity::Warning(), info = Severity::Info();
    EXPECT_TRUE(info < err);
    EXPECT_TRUE(warning > info);
    EXPECT_TRUE(info <= warning);
    EXPECT_TRUE(warning <= warning);
    EXPECT_TRUE(err >= info);
    EXPECT_TRUE(err >= err);
    EXPECT_TRUE(info != err);
    EXPECT_TRUE(warning != info);
    EXPECT_FALSE(info > err);
    EXPECT_FALSE(warning < info);
    EXPECT_FALSE(info >= warning);
    EXPECT_FALSE(err <= info);
}

TEST(SeverityTests, StringRepr) {
    EXPECT_EQ(Severity::Error().ToString(), "[Error]");
    EXPECT_EQ(Severity::Warning().ToString(), "[Warning]");
    EXPECT_EQ(Severity::Info().ToString(), "[Info]");
}
