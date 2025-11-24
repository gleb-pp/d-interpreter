#pragma once
#include "dinterp/locators/locator.h"
#include "dinterp/runtime/values.h"
#include "runtimeContext.h"
#include "dinterp/syntax.h"
#include "varScopes.h"

namespace interp {

class UnaryOpExecutor : public ast::IASTVisitor {
    RuntimeContext& context;
    std::shared_ptr<ScopeStack> scopes;
    std::shared_ptr<runtime::RuntimeValue> curValue;
    locators::SpanLocator curPos;
    void AccessFieldByIndex(const runtime::RuntimeValue& index, const locators::SpanLocator& accessorPos);

public:
    UnaryOpExecutor(RuntimeContext& context, const std::shared_ptr<ScopeStack>& scopes,
                    const std::shared_ptr<runtime::RuntimeValue> curValue, const locators::SpanLocator& curPos);
    const std::shared_ptr<runtime::RuntimeValue>& Value() const;
    const locators::SpanLocator& Position() const;
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
    virtual ~UnaryOpExecutor() override = default;
};

}  // namespace interp
