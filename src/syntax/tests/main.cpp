#include <gtest/gtest.h>

#include <memory>

#include "dinterp/syntax.h"
#include "fixture.h"
using namespace std;
using namespace dinterp;

TEST_F(FileSample, Basic01) {
    ReadFile("basic/01.d", true);
    ASSERT_EQ(program->statements.size(), 2);
}
TEST_F(FileSample, Basic02) {
    ReadFile("basic/02.d", true);
    ASSERT_EQ(program->statements.size(), 3);
    ASSERT_NE(dynamic_pointer_cast<ast::AssignStatement>(program->statements[1]), nullptr);
}
TEST_F(FileSample, Basic03) {
    ReadFile("basic/03.d", true);
    ASSERT_EQ(program->statements.size(), 2);
    auto ifelse = dynamic_pointer_cast<ast::IfStatement>(program->statements[1]);
    ASSERT_NE(ifelse, nullptr);
    ASSERT_TRUE(ifelse->doIfFalse.has_value());
    auto doelse = *ifelse->doIfFalse;
    ASSERT_EQ(doelse->statements.size(), 1);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(doelse->statements[0]);
    ASSERT_EQ(print->pos.Excerpt(), "print \"big\"");
    ASSERT_EQ(print->expressions.size(), 1);
    auto big_expr = print->expressions[0];
}
TEST_F(FileSample, Basic04) {
    ReadFile("basic/04.d", true);
    ASSERT_EQ(program->statements.size(), 2);
    auto ifstat = dynamic_pointer_cast<ast::ShortIfStatement>(program->statements[1]);
    ASSERT_NE(ifstat, nullptr);
    auto print = dynamic_pointer_cast<ast::PrintStatement>(ifstat->doIfTrue);
    ASSERT_NE(print, nullptr);
    ASSERT_EQ(print->pos.Excerpt(), "print \"equal\"");
    ASSERT_EQ(print->expressions.size(), 1);
}
TEST_F(FileSample, Basic05) { ReadFile("basic/05.d", true); }
TEST_F(FileSample, Basic06) { ReadFile("basic/06.d", true); }
TEST_F(FileSample, Basic07) {
    ReadFile("basic/07.d", true);
    ASSERT_EQ(program->statements.size(), 1);
    auto forst = dynamic_pointer_cast<ast::ForStatement>(program->statements[0]);
    ASSERT_TRUE(forst->optVariableName.has_value());
    ASSERT_EQ(forst->optVariableName.value()->identifier, "i");
    ASSERT_EQ(forst->startOrList->pos.Excerpt(), "1");
    ASSERT_TRUE(forst->end.has_value());
    ASSERT_EQ(forst->end.value()->pos.Excerpt(), "3");
}
TEST_F(FileSample, Basic08) {
    ReadFile("basic/08.d", true);
    ASSERT_EQ(program->statements.size(), 2);
    auto forst = dynamic_pointer_cast<ast::ForStatement>(program->statements[1]);
    ASSERT_NE(forst, nullptr);
    ASSERT_TRUE(forst->optVariableName.has_value());
    ASSERT_FALSE(forst->end.has_value());
}
TEST_F(FileSample, Basic09) { ReadFile("basic/09.d", true); }
TEST_F(FileSample, Basic10) {
    ReadFile("basic/10.d", true);
    ASSERT_EQ(program->statements.size(), 2);
    auto var = dynamic_pointer_cast<ast::VarStatement>(program->statements[0]);
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(var->definitions.size(), 1);
    auto optexpr = var->definitions[0].second;
    ASSERT_TRUE(optexpr.has_value());
    auto expr = *optexpr;
    ASSERT_EQ(expr->pos.Excerpt(), "{a:=1, b:=\"hi\"}");
}
TEST_F(FileSample, Basic11) { ReadFile("basic/11.d", true); }
TEST_F(FileSample, Basic12) { ReadFile("basic/12.d", true); }
TEST_F(FileSample, Basic13) { ReadFile("basic/13.d", true); }
TEST_F(FileSample, Basic14) { ReadFile("basic/14.d", true); }
TEST_F(FileSample, Basic15) { ReadFile("basic/15.d", true); }

TEST_F(FileSample, Complex01) { ReadFile("complex/01.d", true); }
TEST_F(FileSample, Complex02) { ReadFile("complex/02.d", true); }
TEST_F(FileSample, Complex03) { ReadFile("complex/03.d", true); }
TEST_F(FileSample, Complex04) { ReadFile("complex/04.d", true); }
TEST_F(FileSample, Complex05) {
    ReadFile("complex/05.d", false);
    ExpectFailure(3, 16);
}
TEST_F(FileSample, Complex06) { ReadFile("complex/06.d", true); }
TEST_F(FileSample, Complex07) { ReadFile("complex/07.d", true); }
TEST_F(FileSample, Complex08) { ReadFile("complex/08.d", true); }
TEST_F(FileSample, Complex09) { ReadFile("complex/09.d", true); }
TEST_F(FileSample, Complex10) { ReadFile("complex/10.d", true); }
TEST_F(FileSample, Complex11) { ReadFile("complex/11.d", true); }
TEST_F(FileSample, Complex12) { ReadFile("complex/12.d", true); }
TEST_F(FileSample, Complex13) { ReadFile("complex/13.d", true); }
TEST_F(FileSample, Complex14) { ReadFile("complex/14.d", true); }
TEST_F(FileSample, Complex15) { ReadFile("complex/15.d", true); }

TEST_F(FileSample, CustomEmptyVar) {
    ReadFile("custom/empty-var.d", false);
    ExpectFailure(0, 0);
    ExpectFailure(0, 3);
}
TEST_F(FileSample, CustomNewlines) { ReadFile("custom/newlines.d", true); }

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
