#include <gtest/gtest.h>

#include <memory>

#include "bigint.h"
#include "fixture.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"
#include "syntaxext/precomputed.h"
using namespace std;

#define DCAST(type, ptr) dynamic_pointer_cast<type>(ptr)

TEST_F(FileSample, Demo01) {
    ReadFile("demos/01.d", true);
    ExpectFailure(2, 7, "WhileConditionFalseAtStart");
    ExpectFailure(4, 10, "CodeUnreachable");
    ExpectFailure(4, 0, "alwaystrue");
}

TEST_F(FileSample, Demo02) {
    ReadFile("demos/02.d", false);
    ExpectFailure(1, 14, "divisionby0");
}

TEST_F(FileSample, Demo03) {
    ReadFile("demos/03.d", false);
    ExpectFailure(1, 25, "string");
    ExpectFailure(1, 25, "int");
}

TEST_F(FileSample, Demo04) {
    ReadFile("demos/04.d", false);
    ExpectFailure(3, 3, "WrongArgumentCount");
}

TEST_F(FileSample, Demo05) {
    ReadFile("demos/05.d", true);
    auto optexpr = DCAST(ast::VarStatement, program->statements[1])->definitions[0].second;
    ASSERT_TRUE(optexpr.has_value());
    auto closure = DCAST(ast::ClosureDefinition, *optexpr);
    EXPECT_EQ(closure->CapturedExternals, vector<string>({"FLAG"}));
    EXPECT_EQ(closure->Params, vector<string>());
    EXPECT_TRUE(closure->Type->StrictTypeEq(runtime::FuncType(false, 0, make_shared<runtime::NoneType>())));
}

TEST_F(FileSample, Demo06) {
    ReadFile("demos/06.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[9]);
    auto sum = DCAST(ast::Sum, print->expressions[0]);
    auto val = dynamic_pointer_cast<runtime::IntegerValue>(DCAST(ast::PrecomputedValue, sum->terms[0])->Value);
    EXPECT_EQ(val->Value(), BigInt("1" + string(100, '0'), 10) + BigInt(5));
}

TEST_F(FileSample, Demo07) {
    ReadFile("demos/07.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[1]);
    auto unaryor = DCAST(ast::OrOperator, print->expressions[0]);
    ASSERT_EQ(unaryor->operands.size(), 1);
    ASSERT_EQ(unaryor->operands[0]->pos.Excerpt(), "f()");
    ExpectFailure(11, 26, "CodeUnreachable");
    print = DCAST(ast::PrintStatement, program->statements[2]);
    auto val = dynamic_pointer_cast<runtime::BoolValue>(DCAST(ast::PrecomputedValue, print->expressions[0])->Value);
    ASSERT_FALSE(val->Value());
}

TEST_F(FileSample, Demo08) {
    ReadFile("demos/08.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[2]);
    auto x = DCAST(ast::XorOperator, print->expressions[0]);
    ASSERT_EQ(x->operands.size(), 3);
    ASSERT_FALSE(DCAST(runtime::BoolValue, DCAST(ast::PrecomputedValue, x->operands[0])->Value)->Value());
}

TEST_F(FileSample, Demo09) {
    ReadFile("demos/09.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[2]);
    auto x = DCAST(ast::Sum, print->expressions[0]);
    ASSERT_EQ(x->terms.size(), 2);
    ASSERT_EQ(x->operators, vector<ast::Sum::SumOperator>({ast::Sum::SumOperator::Plus}));
    ASSERT_EQ(DCAST(runtime::IntegerValue, DCAST(ast::PrecomputedValue, x->terms[0])->Value)->Value(), BigInt(5));
    ASSERT_EQ(DCAST(ast::PrimaryIdent, x->terms[1])->name->identifier, "a");
}

TEST_F(FileSample, Demo10) {
    ReadFile("demos/10.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[3]);
    auto x = DCAST(ast::Sum, print->expressions[0]);
    ASSERT_EQ(x->terms.size(), 3);
    ASSERT_EQ(x->operators, vector<ast::Sum::SumOperator>(2, ast::Sum::SumOperator::Plus));
    ASSERT_EQ(DCAST(runtime::StringValue, DCAST(ast::PrecomputedValue, x->terms[0])->Value)->Value(), "http://");
    ASSERT_EQ(DCAST(ast::PrimaryIdent, x->terms[1])->name->identifier, "c");
    ASSERT_EQ(DCAST(runtime::StringValue, DCAST(ast::PrecomputedValue, x->terms[2])->Value)->Value(), ".com/");
}

TEST_F(FileSample, Demo11) {
    ReadFile("demos/11.d", true);
    ExpectFailure(8, 15, "CodeUnreachable");
    EXPECT_EQ(
        dynamic_pointer_cast<ast::BinaryRelation>(DCAST(ast::PrintStatement, program->statements[2])->expressions[0])
            ->operands.size(),
        3);
    ASSERT_FALSE(
        dynamic_pointer_cast<runtime::BoolValue>(dynamic_pointer_cast<ast::PrecomputedValue>(
                                                     DCAST(ast::PrintStatement, program->statements[3])->expressions[0])
                                                     ->Value)
            ->Value());
}

TEST_F(FileSample, Demo12) {
    ReadFile("demos/12.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[1]);
#define EXPECT_INDEX_EQ(i, runtimeType, val) \
    EXPECT_EQ(DCAST(runtime::runtimeType, DCAST(ast::PrecomputedValue, print->expressions[i])->Value)->Value(), val)
    EXPECT_INDEX_EQ(0, IntegerValue, BigInt(123));
    EXPECT_INDEX_EQ(1, IntegerValue, BigInt(124));
    EXPECT_INDEX_EQ(2, IntegerValue, BigInt(123));
    EXPECT_NEAR(DCAST(runtime::RealValue, DCAST(ast::PrecomputedValue, print->expressions[3])->Value)->Value(), 0.45l,
                1e-9);
    print = DCAST(ast::PrintStatement, program->statements[2]);
    EXPECT_INDEX_EQ(0, IntegerValue, BigInt(7));
    print = DCAST(ast::PrintStatement, program->statements[3]);
    EXPECT_TRUE(!!DCAST(ast::Unary, print->expressions[0]));
    print = DCAST(ast::PrintStatement, program->statements[4]);
    EXPECT_TRUE(!!DCAST(ast::Unary, print->expressions[0]));
#undef EXPECT_INDEX_EQ
}

TEST_F(FileSample, Demo13) {
    ReadFile("demos/13.d", true);
    ExpectFailure(7, 11, "IntegerZeroDivisionWarning");
}

TEST_F(FileSample, Demo14) {
    ReadFile("demos/14.d", true);
    ExpectFailure(3, 0, "sideeffects");
    ExpectFailure(4, 0, "sideeffects");
}

TEST_F(FileSample, Demo15) {
    ReadFile("demos/15.d", true);
    auto func =
        DCAST(ast::ClosureDefinition, DCAST(ast::VarStatement, program->statements[1])->definitions[0].second.value());
    EXPECT_TRUE(func->CapturedExternals.empty());
    EXPECT_FALSE(func->Type->Pure());
    ASSERT_EQ(program->statements.size(), 5);  // var, var, call, condition, print
    EXPECT_TRUE(!!DCAST(ast::ExpressionStatement, program->statements[3]));
    EXPECT_TRUE(!!DCAST(ast::PrintStatement, DCAST(ast::Body, program->statements[4])->statements[0]));
}

TEST_F(FileSample, Demo16) {
    ReadFile("demos/16.d", false);
    ExpectFailure(8, 10, "NoneValueAccessed");
}

TEST_F(FileSample, Demo17) {
    ReadFile("demos/17.d", false);
    ExpectFailure(14, 6, "VariableNotDefined");
}

TEST_F(FileSample, Demo18) {
    ReadFile("demos/18.d", false);
    ExpectFailure(1, 25, "DuplicateFieldNames");
}

TEST_F(FileSample, Demo19) {
    ReadFile("demos/19.d", true);
    ExpectFailure(4, 4, "VariableNeverUsed");
}

TEST_F(FileSample, Demo20) {
    ReadFile("demos/20.d", true);
    ExpectFailure(4, 9, "AssignedValueUnused");
}

TEST_F(FileSample, Demo21) {
    ReadFile("demos/21.d", true);
    ExpectFailure(9, 31, "Unreachable");
    ExpectFailure(13, 31, "Unreachable");
    ExpectFailure(15, 31, "Unreachable");
    auto funcbody = DCAST(
        ast::LongFuncBody,
        DCAST(ast::ClosureDefinition, DCAST(ast::VarStatement, program->statements[0])->definitions[0].second.value())
            ->Definition);
    auto loopbody = DCAST(ast::ForStatement, funcbody->funcBody->statements[1])->action;
    ASSERT_EQ(loopbody->statements.size(), 1);
    auto ifstat = DCAST(ast::IfStatement, loopbody->statements[0]);
    auto iftrue = ifstat->doIfTrue;
    ASSERT_EQ(iftrue->statements.size(), 3);
    ASSERT_TRUE(!!DCAST(ast::ReturnStatement, iftrue->statements[2]));
    auto iffalse = *ifstat->doIfFalse;
    ASSERT_EQ(iffalse->statements.size(), 1);
    ASSERT_TRUE(!!DCAST(ast::ExitStatement, iffalse->statements[0]));
    ASSERT_EQ(funcbody->funcBody->statements.size(), 3);
}

TEST_F(FileSample, Demo22) {
    ReadFile("demos/22.d", true);
    ASSERT_EQ(program->statements.size(), 2);
    auto print = DCAST(ast::PrintStatement, DCAST(ast::Body, program->statements[1])->statements[0]);
    ASSERT_EQ(print->expressions[0]->pos.Excerpt(), "\"ok\"");
}

TEST_F(FileSample, Demo23) {
    ReadFile("demos/23.d", true);
    ASSERT_TRUE(!!DCAST(ast::IfStatement, program->statements[2]));
}

TEST_F(FileSample, Demo24) {
    ReadFile("demos/24.d", true);
    ExpectFailure(1, 8, "false");
    EXPECT_TRUE(program->statements.empty());
}

TEST_F(FileSample, Demo25) {
    ReadFile("demos/25.d", false);
    ExpectFailure(3, 6, "WhileConditionNotBoolAtStart");
}

TEST_F(FileSample, Demo26) {
    ReadFile("demos/26.d", true);
    auto print = DCAST(ast::PrintStatement, program->statements[2]);
    ASSERT_EQ(print->expressions.size(), 1);
    auto expr = print->expressions[0];
    ASSERT_TRUE(!!DCAST(ast::Unary, expr));
    print = DCAST(ast::PrintStatement, program->statements[5]);
    ASSERT_EQ(print->expressions.size(), 1);
    expr = print->expressions[0];
    ASSERT_TRUE(DCAST(runtime::BoolValue, DCAST(ast::PrecomputedValue, expr)->Value)->Value());
}

TEST_F(FileSample, Demo27) {
    ReadFile("demos/27.d", false);
    ExpectFailure(14, 6, "VariableNotDefined");
}

TEST_F(FileSample, Demo28) {
    ReadFile("demos/28.d", false);
    ExpectFailure(1, 9, "real");
}

TEST_F(FileSample, Demo29) {
    ReadFile("demos/29.d", false);
    ExpectFailure(4, 9, "string");
}

TEST_F(FileSample, Demo30) {
    ReadFile("demos/30.d", false);
    ExpectFailure(6, 20, "cycle");
}

TEST_F(FileSample, Demo31) {
    ReadFile("demos/31.d", false);
    ExpectFailure(11, 4, "unction");
}

TEST_F(FileSample, Demo32) {
    ReadFile("demos/32.d", false);
    ExpectFailure(10, 6, "(pure)function");
}

TEST_F(FileSample, Demo33) {
    ReadFile("demos/33.d", false);
    ExpectFailure(9, 9, "tuple");
}

TEST_F(FileSample, Demo34) {
    ReadFile("demos/34.d", false);
    ExpectFailure(9, 5, "array");
}

TEST_F(FileSample, Demo35) { ReadFile("demos/35.d", true); }

TEST_F(FileSample, Demo36) {
    ReadFile("demos/36.d", true);
    auto funcexpr = DCAST(ast::Unary, DCAST(ast::ExpressionStatement, program->statements[1])->expr);
    ASSERT_TRUE(funcexpr->prefixOps.empty());
    ASSERT_EQ(funcexpr->postfixOps.size(), 1);
    auto funcbody = DCAST(ast::LongFuncBody, DCAST(ast::ClosureDefinition, funcexpr->expr)->Definition)->funcBody;
    ASSERT_TRUE(!!DCAST(ast::IfStatement, funcbody->statements[0]));
}

TEST_F(FileSample, Demo37) {
    ReadFile("demos/37.d", true);
    ASSERT_EQ(program->statements.size(), 3);
    ASSERT_TRUE(!!dynamic_pointer_cast<ast::ExpressionStatement>(program->statements[0]));
    string body = program->statements[1]->pos.Excerpt();
    {
        size_t start = body.find_first_not_of("\n ");
        size_t end = body.find_last_not_of("\n ");
        body = body.substr(start, end - start + 1);
    }
    ASSERT_EQ(body, "print \"ok\"");
    ASSERT_TRUE(!!dynamic_pointer_cast<ast::VarStatement>(program->statements[2]));
}

TEST_F(FileSample, Demo38) {
    ReadFile("demos/38.d", false);
    ExpectFailure(1, 14, "WrongArgumentCount");
}

TEST_F(FileSample, Demo39) {
    ReadFile("demos/39.d", true);
    ExpectFailure(2, 4, "AssignedValueUnused");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
