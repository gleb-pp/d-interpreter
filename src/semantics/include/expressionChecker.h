#pragma once
#include "complog/CompilationLog.h"
#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"
#include "valueTimeline.h"
#include <vector>

// may modify the syntax tree
class ExpressionChecker : public ast::IASTVisitor {
    complog::ICompilationLog& log;
    bool pure;
    std::optional<runtime::TypeOrValue> res;
    std::optional<std::shared_ptr<ast::ASTNode>> replacement;
    ValueTimeline& values;

    void VisitAndOrOperator(bool isOr, std::vector<std::shared_ptr<ast::Expression>>& operands,
                            const locators::SpanLocator& position);

public:
    ExpressionChecker(complog::ICompilationLog& log, ValueTimeline& values);
    bool HasResult() const;  // false in case of error
    bool Pure() const;
    runtime::TypeOrValue Result() const;
    std::optional<std::shared_ptr<ast::ASTNode>> Replacement() const;
    std::shared_ptr<ast::Expression> AssertReplacementAsExpression() const;
    ValueTimeline& ProgramState() const;
    void VisitBody(ast::Body& node) override;
    void VisitVarStatement(ast::VarStatement& node) override;
    void VisitIfStatement(ast::IfStatement& node) override;
    void VisitShortIfStatement(ast::ShortIfStatement& node) override;
    void VisitWhileStatement(ast::WhileStatement& node) override;
    void VisitForStatement(ast::ForStatement& node) override;
    void VisitLoopStatement(ast::LoopStatement& node) override;
    void VisitExitStatement(ast::ExitStatement& node) override;
    void VisitAssignStatement(ast::AssignStatement& node) override;
    void VisitPrintStatement(ast::PrintStatement& node) override;
    void VisitReturnStatement(ast::ReturnStatement& node) override;
    void VisitExpressionStatement(ast::ExpressionStatement& node) override;
    void VisitCommaExpressions(ast::CommaExpressions& node) override;
    void VisitCommaIdents(ast::CommaIdents& node) override;
    void VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) override;
    void VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) override;
    void VisitParenMemberAccessor(ast::ParenMemberAccessor& node) override;
    void VisitIndexAccessor(ast::IndexAccessor& node) override;
    void VisitReference(ast::Reference& node) override;
    void VisitXorOperator(ast::XorOperator& node) override;
    void VisitOrOperator(ast::OrOperator& node) override;
    void VisitAndOperator(ast::AndOperator& node) override;
    void VisitBinaryRelation(ast::BinaryRelation& node) override;
    void VisitSum(ast::Sum& node) override;
    void VisitTerm(ast::Term& node) override;
    void VisitUnary(ast::Unary& node) override;
    void VisitUnaryNot(ast::UnaryNot& node) override;
    void VisitPrefixOperator(ast::PrefixOperator& node) override;
    void VisitTypecheckOperator(ast::TypecheckOperator& node) override;
    void VisitCall(ast::Call& node) override;
    void VisitAccessorOperator(ast::AccessorOperator& node) override;
    void VisitPrimaryIdent(ast::PrimaryIdent& node) override;
    void VisitParenthesesExpression(ast::ParenthesesExpression& node) override;
    void VisitTupleLiteralElement(ast::TupleLiteralElement& node) override;
    void VisitTupleLiteral(ast::TupleLiteral& node) override;
    void VisitShortFuncBody(ast::ShortFuncBody& node) override;
    void VisitLongFuncBody(ast::LongFuncBody& node) override;
    void VisitFuncLiteral(ast::FuncLiteral& node) override;
    void VisitTokenLiteral(ast::TokenLiteral& node) override;
    void VisitArrayLiteral(ast::ArrayLiteral& node) override;
    void VisitCustom(ast::ASTNode& node) override;
    virtual ~ExpressionChecker() = default;
};
