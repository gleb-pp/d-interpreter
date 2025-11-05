#pragma once
#include "complog/CompilationLog.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"
#include "valueTimeline.h"

// may modify the syntax tree
class StatementChecker : public ast::IASTVisitor {
public:
    enum class TerminationKind { ReachedEnd, Exited, Returned, Errored };

private:
    complog::ICompilationLog& log;
    bool pure;
    std::optional<std::shared_ptr<runtime::Type>> returned;
    std::optional<std::vector<std::shared_ptr<ast::Statement>>> replacement;
    ValueTimeline& values;
    bool inFunction, inCycle;
    TerminationKind terminationKind;

    void VisitLoopBody(

public:
    StatementChecker(complog::ICompilationLog& log, ValueTimeline& values, bool inFunction, bool inCycle);
    bool Pure() const;
    std::optional<std::shared_ptr<runtime::Type>> Returned() const;
    ValueTimeline& ProgramState() const;
    TerminationKind Terminated() const;
    const std::optional<std::vector<std::shared_ptr<ast::Statement>>>& Replacement() const;
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
    virtual ~StatementChecker() = default;
};
