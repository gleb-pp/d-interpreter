#pragma once
#include "complog/CompilationLog.h"
#include "syntax.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "valueTimeline.h"

// may modify the syntax tree
class StatementChecker : public ast::IASTVisitor {
    complog::ICompilationLog& log;
    bool pure;
    std::optional<std::variant<std::shared_ptr<runtime::Type>, std::shared_ptr<runtime::RuntimeValue>>> returned;
    ValueTimeline values;
    bool inFunction, inCycle;
    enum class TerminationKind {
        ReachedEnd, Exited, Returned, Errored
    };
    TerminationKind terminationKind;

public:
    StatementChecker(complog::ICompilationLog& log, const ValueTimeline& values, bool inFunction, bool inCycle);
    bool Pure() const;
    std::optional<std::variant<std::shared_ptr<runtime::Type>, std::shared_ptr<runtime::RuntimeValue>>> Result() const;
    const ValueTimeline& ProgramState() const;
    TerminationKind Terminated() const;
    virtual void VisitBody(ast::Body& node) = 0;
    virtual void VisitVarStatement(ast::VarStatement& node) = 0;
    virtual void VisitIfStatement(ast::IfStatement& node) = 0;
    virtual void VisitShortIfStatement(ast::ShortIfStatement& node) = 0;
    virtual void VisitWhileStatement(ast::WhileStatement& node) = 0;
    virtual void VisitForStatement(ast::ForStatement& node) = 0;
    virtual void VisitLoopStatement(ast::LoopStatement& node) = 0;
    virtual void VisitExitStatement(ast::ExitStatement& node) = 0;
    virtual void VisitAssignStatement(ast::AssignStatement& node) = 0;
    virtual void VisitPrintStatement(ast::PrintStatement& node) = 0;
    virtual void VisitReturnStatement(ast::ReturnStatement& node) = 0;
    virtual void VisitExpressionStatement(ast::ExpressionStatement& node) = 0;
    virtual void VisitCommaExpressions(ast::CommaExpressions& node) = 0;
    virtual void VisitCommaIdents(ast::CommaIdents& node) = 0;
    virtual void VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) = 0;
    virtual void VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) = 0;
    virtual void VisitParenMemberAccessor(ast::ParenMemberAccessor& node) = 0;
    virtual void VisitIndexAccessor(ast::IndexAccessor& node) = 0;
    virtual void VisitReference(ast::Reference& node) = 0;
    virtual void VisitExpression(ast::Expression& node) = 0;
    virtual void VisitOrOperator(ast::OrOperator& node) = 0;
    virtual void VisitAndOperator(ast::AndOperator& node) = 0;
    virtual void VisitBinaryRelation(ast::BinaryRelation& node) = 0;
    virtual void VisitSum(ast::Sum& node) = 0;
    virtual void VisitTerm(ast::Term& node) = 0;
    virtual void VisitUnary(ast::Unary& node) = 0;
    virtual void VisitUnaryNot(ast::UnaryNot& node) = 0;
    virtual void VisitPrefixOperator(ast::PrefixOperator& node) = 0;
    virtual void VisitTypecheckOperator(ast::TypecheckOperator& node) = 0;
    virtual void VisitCall(ast::Call& node) = 0;
    virtual void VisitAccessorOperator(ast::AccessorOperator& node) = 0;
    virtual void VisitPrimaryIdent(ast::PrimaryIdent& node) = 0;
    virtual void VisitParenthesesExpression(ast::ParenthesesExpression& node) = 0;
    virtual void VisitTupleLiteralElement(ast::TupleLiteralElement& node) = 0;
    virtual void VisitTupleLiteral(ast::TupleLiteral& node) = 0;
    virtual void VisitShortFuncBody(ast::ShortFuncBody& node) = 0;
    virtual void VisitLongFuncBody(ast::LongFuncBody& node) = 0;
    virtual void VisitFuncLiteral(ast::FuncLiteral& node) = 0;
    virtual void VisitTokenLiteral(ast::TokenLiteral& node) = 0;
    virtual void VisitArrayLiteral(ast::ArrayLiteral& node) = 0;
    virtual void VisitCustom(ast::ASTNode& node);
    virtual ~StatementChecker() = default;
};
