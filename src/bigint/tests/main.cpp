#include <gtest/gtest.h>

#include <cmath>

#include "arith_samples.h"
#include "bigint.h"
using namespace std;

TEST(Repr, ConsStr) {
    EXPECT_EQ(BigInt("1239423", 10), 1239423);
    EXPECT_EQ(BigInt("00", 10), 0);
    EXPECT_EQ(BigInt("00", 2), 0);
    EXPECT_EQ(BigInt("10010011", 2), 0b10010011);
    EXPECT_EQ(BigInt("12021101", 3), 3844);
    EXPECT_EQ(BigInt("392", 16), 0x392);
    EXPECT_EQ(BigInt("abcdef19230134", 16), 0xabcdef19230134);
    auto googol = BigInt(
        "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 10);
    EXPECT_EQ(googol.Repr(1ul << 32), (vector<size_t>{4681, 2904921283, 2095778599, 2227490315, 4085960256, 2384534140,
                                                      2863809288, 2821623568, 0, 0, 0}));
}

TEST(Arith, Mul) {
    // (1x + 8)(2x + 3) = 2xx + 19x + 24
    BigInt a(vector<size_t>{1, 8}, 1ul << 32), b(vector<size_t>{2, 3}, 1ul << 32);
    ASSERT_EQ(a * b, BigInt(vector<size_t>{2, 19, 24}, 1ul << 32));
}

TEST(Arith, Div) {
    // (8x + 4) / (3x + 2) = 2 + (2x) / (3x + 2)
    BigInt a(vector<size_t>{8, 4}, 1ul << 32), b(vector<size_t>{3, 2}, 1ul << 32);
    EXPECT_EQ(a.DivLeaveMod(b), 2);
    EXPECT_EQ(a, BigInt(vector<size_t>{2, 0}, 1ul << 32));
}

TEST(Arith, Fac100) {
    BigInt res = 1l;
    for (long i = 2; i <= 100; i++) res *= i;
    ASSERT_EQ(res.ToString(), string("93326215443944152681699238856266700490715968264381621468592963895217599993229") +
                                  "915608941463976156518286253697920827223758251185210916864000000000000000000000000");
}

TEST(Repr, Float) {
    EXPECT_EQ(BigInt(0.8l), 0);
    EXPECT_EQ(BigInt(-0.8l), 0);
    EXPECT_EQ(BigInt(8.8l), 8);
    EXPECT_EQ(BigInt(8.0l), 8);
    EXPECT_EQ(BigInt(-8.0l), -8);
    string googol = BigInt(1e100l).ToString();
    EXPECT_TRUE(googol.size() == 100 || googol.size() == 101);
    long double lgoogol;
    {
        stringstream ss(googol);
        ss >> lgoogol;
    }
    EXPECT_NEAR(lgoogol, 1e100l, 1e90l);
    EXPECT_EQ(BigInt(1e-100l), 0);
    EXPECT_EQ(BigInt(-1e-100l), 0);
    googol = BigInt(-1e100l).ToString();
    EXPECT_TRUE(googol.size() == 100 || googol.size() == 101);
    {
        stringstream ss(googol);
        ss >> lgoogol;
    }
    EXPECT_NEAR(lgoogol, -1e100l, 1e90l);
    EXPECT_EQ(BigInt((long double)INFINITY), 0);
    EXPECT_EQ(BigInt((long double)NAN), 0);

    EXPECT_NEAR(BigInt("1" + string(100, '0'), 10).ToFloat(), 1e100, 1e90);
    EXPECT_NEAR(BigInt("-1" + string(100, '0'), 10).ToFloat(), -1e100, 1e90);
    EXPECT_EQ(BigInt(102).ToFloat(), 102.0);
    EXPECT_EQ((BigInt(1ul << 45) * BigInt(1ul << 45)).ToFloat(), 1237940039285380274899124224.0);
}

class ArithSamples : public testing::Test {
protected:
    BigInt ints[20];
    void SetUp() override {
        for (int i = 0; i < 20; i++) ints[i] = BigInt(sample_numbers[i], 10);
    }
};

TEST_F(ArithSamples, Add) {
    int k = 0;
    for (int i = 0; i < 20; i++) {
        BigInt a = ints[i];
        for (int j = 0; j < 20; j++) {
            EXPECT_EQ((a + ints[j]).ToString(), sums[k]);
            EXPECT_EQ((a + ints[j]), BigInt(sums[k], 10));
            ++k;
        }
    }
}

TEST_F(ArithSamples, Sub) {
    int k = 0;
    for (int i = 0; i < 20; i++) {
        BigInt a = ints[i];
        for (int j = 0; j < 20; j++) {
            EXPECT_EQ((a - ints[j]).ToString(), diffs[k]);
            EXPECT_EQ((a - ints[j]), BigInt(diffs[k], 10));
            ++k;
        }
    }
}

TEST_F(ArithSamples, Mul) {
    int k = 0;
    for (int i = 0; i < 20; i++) {
        BigInt a = ints[i];
        for (int j = 0; j < 20; j++) {
            EXPECT_EQ((a * ints[j]).ToString(), muls[k]);
            EXPECT_EQ((a * ints[j]), BigInt(muls[k], 10));
            ++k;
        }
    }
}

TEST_F(ArithSamples, Div) {
    int k = 0;
    for (int i = 0; i < 20; i++) {
        BigInt a = ints[i];
        for (int j = 0; j < 20; j++) {
            EXPECT_EQ((a / ints[j]).ToString(), divs[k]);
            EXPECT_EQ((a / ints[j]), BigInt(divs[k], 10));
            ++k;
        }
    }
}

TEST_F(ArithSamples, Mod) {
    int k = 0;
    for (int i = 0; i < 20; i++) {
        BigInt a = ints[i];
        for (int j = 0; j < 20; j++) {
            EXPECT_EQ((a % ints[j]).ToString(), mods[k]);
            EXPECT_EQ((a % ints[j]), BigInt(mods[k], 10));
            ++k;
        }
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
