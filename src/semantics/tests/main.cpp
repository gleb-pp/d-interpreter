#include <gtest/gtest.h>

#include <memory>

#include "bigint.h"
#include "fixture.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"
#include "semantic.h"
#include "syntaxext/precomputed.h"
using namespace std;

TEST_F(FileSample, Demo01) {
    ReadFile("demos/01.d", true);
    ExpectFailure(2, 7, "WhileConditionFalseAtStart");
    ExpectFailure(4, 10, "CodeUnreachable");
    ExpectFailure(4, 0, "always true");
}

TEST_F(FileSample, Demo02) {
    ReadFile("demos/02.d", false);
    ExpectFailure(1, 13, "zero");
}

TEST_F(FileSample, Demo03) {
    ReadFile("demos/03.d", false);
    ExpectFailure(1, 25, "WrongArgumentType");
    ExpectFailure(1, 25, "string");
    ExpectFailure(1, 25, "int");
}

TEST_F(FileSample, Demo04) {
    ReadFile("demos/04.d", false);
    ExpectFailure(3, 3, "WrongArgumentCount");
}

TEST_F(FileSample, Demo05) {
    ReadFile("demos/05.d", true);
    auto optexpr = dynamic_pointer_cast<ast::VarStatement>(program->statements[1])->definitions[0].second;
    ASSERT_TRUE(optexpr.has_value());
    auto closure = dynamic_pointer_cast<ast::ClosureDefinition>(*optexpr);
    EXPECT_EQ(closure->CapturedExternals, vector<string>({"FLAG"}));
    EXPECT_EQ(closure->Params, vector<string>());
    EXPECT_TRUE(closure->Type->StrictTypeEq(runtime::FuncType(false, 0, make_shared<runtime::NoneType>())));
}

TEST_F(FileSample, Demo06) {
    ReadFile("demos/06.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[9]);
    auto sum = dynamic_pointer_cast<ast::Sum>(print->expressions[0]);
    auto val = dynamic_pointer_cast<runtime::IntegerValue>(
        dynamic_pointer_cast<ast::PrecomputedValue>(sum->terms[0])->Value);
    EXPECT_EQ(val->Value(), BigInt("1" + string(100, '0'), 10) + BigInt(5));
}

TEST_F(FileSample, Demo07) {
    ReadFile("demos/07.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[1]);
    auto unaryor = dynamic_pointer_cast<ast::OrOperator>(print->expressions[0]);
    ASSERT_EQ(unaryor->operands.size(), 1);
    ASSERT_EQ(unaryor->operands[0]->pos.Excerpt(), "f()");
    ExpectFailure(11, 26, "CodeUnreachable");
    print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[2]);
    auto val = dynamic_pointer_cast<runtime::BoolValue>(
        dynamic_pointer_cast<ast::PrecomputedValue>(print->expressions[0])->Value);
    ASSERT_FALSE(val->Value());
}

TEST_F(FileSample, Demo08) {
    ReadFile("demos/08.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[2]);
    auto x = dynamic_pointer_cast<ast::XorOperator>(print->expressions[0]);
    ASSERT_EQ(x->operands.size(), 3);
    ASSERT_FALSE(
        dynamic_pointer_cast<runtime::BoolValue>(dynamic_pointer_cast<ast::PrecomputedValue>(x->operands[0])->Value)
            ->Value());
}

TEST_F(FileSample, Demo09) {
    ReadFile("demos/09.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[2]);
    auto x = dynamic_pointer_cast<ast::Sum>(print->expressions[0]);
    ASSERT_EQ(x->terms.size(), 2);
    ASSERT_EQ(x->operators, vector<ast::Sum::SumOperator>({ast::Sum::SumOperator::Plus}));
    ASSERT_EQ(
        dynamic_pointer_cast<runtime::IntegerValue>(dynamic_pointer_cast<ast::PrecomputedValue>(x->terms[0])->Value)
            ->Value(),
        BigInt(5));
    ASSERT_EQ(dynamic_pointer_cast<ast::PrimaryIdent>(x->terms[1])->name->identifier, "a");
}

TEST_F(FileSample, Demo10) {
    ReadFile("demos/10.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[2]);
    auto x = dynamic_pointer_cast<ast::Sum>(print->expressions[0]);
    ASSERT_EQ(x->terms.size(), 3);
    ASSERT_EQ(x->operators, vector<ast::Sum::SumOperator>(2, ast::Sum::SumOperator::Plus));
    ASSERT_EQ(
        dynamic_pointer_cast<runtime::StringValue>(dynamic_pointer_cast<ast::PrecomputedValue>(x->terms[0])->Value)
            ->Value(),
        "http://");
    ASSERT_EQ(dynamic_pointer_cast<ast::PrimaryIdent>(x->terms[1])->name->identifier, "c");
    ASSERT_EQ(
        dynamic_pointer_cast<runtime::StringValue>(dynamic_pointer_cast<ast::PrecomputedValue>(x->terms[2])->Value)
            ->Value(),
        ".com/");
}

TEST_F(FileSample, Demo11) {
    ReadFile("demos/11.d", true);
    ExpectFailure(8, 15, "CodeUnreachable");
    EXPECT_EQ(dynamic_pointer_cast<ast::BinaryRelation>(
                  dynamic_pointer_cast<ast::PrintStatement>(program->statements[2])->expressions[0])
                  ->operands.size(),
              3);
    ASSERT_FALSE(dynamic_pointer_cast<runtime::BoolValue>(
                     dynamic_pointer_cast<ast::PrecomputedValue>(
                         dynamic_pointer_cast<ast::PrintStatement>(program->statements[3])->expressions[0])
                         ->Value)
                     ->Value());
}

TEST_F(FileSample, Demo12) {
    ReadFile("demos/12.d", true);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[1]);
#define EXPECT_INDEX_EQ(i, runtimeType, val)                                                 \
    EXPECT_EQ(dynamic_pointer_cast<runtime::runtimeType>(                                    \
                  dynamic_pointer_cast<ast::PrecomputedValue>(print->expressions[i])->Value) \
                  ->Value(),                                                                 \
              val)
    EXPECT_INDEX_EQ(0, IntegerValue, BigInt(123));
    EXPECT_INDEX_EQ(1, IntegerValue, BigInt(124));
    EXPECT_INDEX_EQ(2, IntegerValue, BigInt(123));
    EXPECT_INDEX_EQ(2, RealValue, 0.45l);
    print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[2]);
    EXPECT_INDEX_EQ(0, IntegerValue, BigInt(7));
    print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[3]);
    EXPECT_INDEX_EQ(0, StringValue, "qrxdty");
    print = dynamic_pointer_cast<ast::PrintStatement>(program->statements[4]);
    EXPECT_TRUE(!!dynamic_pointer_cast<ast::Unary>(print->expressions[0]));
}

TEST_F(FileSample, Demo13) {
    ReadFile("demos/13.d", true);
    ExpectFailure(7, 12, "IntegerZeroDivisionWarning");
}

TEST_F(FileSample, Demo14) {
    ReadFile("demos/14.d", true);
    ExpectFailure(3, 0, "side effects");
    ExpectFailure(4, 0, "side effects");
}

TEST_F(FileSample, Demo15) {
    ReadFile("demos/15.d", tf);
}

TEST_F(FileSample, Demo16) {
    ReadFile("demos/16.d", tf);
}

TEST_F(FileSample, Demo17) {
    ReadFile("demos/17.d", tf);
}

TEST_F(FileSample, Demo18) {
    ReadFile("demos/18.d", tf);
}

TEST_F(FileSample, Demo19) {
    ReadFile("demos/19.d", tf);
}

TEST_F(FileSample, Demo20) {
    ReadFile("demos/20.d", tf);
}

TEST_F(FileSample, Demo21) {
    ReadFile("demos/21.d", tf);
}

TEST_F(FileSample, Demo22) {
    ReadFile("demos/22.d", tf);
}

TEST_F(FileSample, Demo23) {
    ReadFile("demos/23.d", tf);
}

TEST_F(FileSample, Demo24) {
    ReadFile("demos/24.d", tf);
}

TEST_F(FileSample, Demo25) {
    ReadFile("demos/25.d", tf);
}

TEST_F(FileSample, Demo26) {
    ReadFile("demos/26.d", tf);
}

TEST_F(FileSample, Demo27) {
    ReadFile("demos/27.d", tf);
}

TEST_F(FileSample, Demo28) {
    ReadFile("demos/28.d", tf);
}

TEST_F(FileSample, Demo29) {
    ReadFile("demos/29.d", tf);
}

TEST_F(FileSample, Demo30) {
    ReadFile("demos/30.d", tf);
}

TEST_F(FileSample, Demo31) {
    ReadFile("demos/31.d", tf);
}

TEST_F(FileSample, Demo32) {
    ReadFile("demos/32.d", tf);
}

TEST_F(FileSample, Demo33) {
    ReadFile("demos/33.d", tf);
}

TEST_F(FileSample, Demo34) {
    ReadFile("demos/34.d", tf);
}

TEST_F(FileSample, Demo35) {
    ReadFile("demos/35.d", tf);
}

TEST_F(FileSample, Demo36) {
    ReadFile("demos/36.d", tf);
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
