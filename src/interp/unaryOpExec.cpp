#include "interp/unaryOpExec.h"
#include <memory>
#include <sstream>
#include "locators/locator.h"
#include "runtime/derror.h"
#include "runtime/values.h"
#include "interp/execution.h"
using namespace std;

namespace interp {

/*
class UnaryOpExecutor : public ast::IASTVisitor {
    RuntimeContext& context;
    std::shared_ptr<ScopeStack> scopes;
    std::shared_ptr<runtime::RuntimeValue> curValue;
    locators::SpanLocator curPos;

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
*/

UnaryOpExecutor::UnaryOpExecutor(RuntimeContext& context, const std::shared_ptr<ScopeStack>& scopes,
                const std::shared_ptr<runtime::RuntimeValue> curValue, const locators::SpanLocator& curPos)
    : context(context), scopes(scopes), curValue(curValue), curPos(curPos) {}

const std::shared_ptr<runtime::RuntimeValue>& UnaryOpExecutor::Value() const {
    return curValue;
}

const locators::SpanLocator& UnaryOpExecutor::Position() const {
    return curPos;
}

#define DISALLOWED_VISIT_FULL(name, classname)                              \
    void UnaryOpExecutor::Visit##name(ast::classname& node) {               \
        throw runtime_error("UnaryOpExecutor cannot visit " #name " node"); \
    }
#define DISALLOWED_VISIT(name) DISALLOWED_VISIT_FULL(name, name)

DISALLOWED_VISIT(Body)
DISALLOWED_VISIT(VarStatement)
DISALLOWED_VISIT(IfStatement)
DISALLOWED_VISIT(ShortIfStatement)
DISALLOWED_VISIT(WhileStatement)
DISALLOWED_VISIT(ForStatement)
DISALLOWED_VISIT(LoopStatement)
DISALLOWED_VISIT(ExitStatement)
DISALLOWED_VISIT(AssignStatement)
DISALLOWED_VISIT(PrintStatement)
DISALLOWED_VISIT(ReturnStatement)
DISALLOWED_VISIT(ExpressionStatement)
DISALLOWED_VISIT(CommaExpressions)
DISALLOWED_VISIT(CommaIdents)

void UnaryOpExecutor::VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) {
    auto res = curValue->Field(node.name->identifier);
    if (!res) {
        context.SetThrowingState(runtime::DRuntimeError("Object (of type \"" + curValue->TypeOfValue()->Name() +
                                                        "\") had no field \"" + node.name->identifier + "\""),
                                 node.pos);
        return;
    }
    if (res->index()) {
        context.SetThrowingState(get<1>(*res), node.pos);
        return;
    }
    curValue = get<0>(*res);
    curPos = locators::SpanLocator(curPos, node.pos);
}

void UnaryOpExecutor::AccessFieldByIndex(const runtime::RuntimeValue& index, const locators::SpanLocator& accessorPos) {
    auto res = curValue->Field(index);
    if (!res) {
        stringstream ss;
        index.PrintSelf(ss);
        context.SetThrowingState(runtime::DRuntimeError("Object (of type \"" + curValue->TypeOfValue()->Name() +
                                                        "\") has no indexed field \"" + ss.str() +
                                                        "\" (index of type \"" + index.TypeOfValue()->Name() + "\")"),
                                 accessorPos);
        return;
    }
    if (res->index()) {
        context.SetThrowingState(get<1>(*res), accessorPos);
        return;
    }
    curValue = get<0>(*res);
    curPos = locators::SpanLocator(curPos, accessorPos);
}

void UnaryOpExecutor::VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) {
    AccessFieldByIndex(runtime::IntegerValue(node.index->value), node.pos);
}

void UnaryOpExecutor::VisitParenMemberAccessor(ast::ParenMemberAccessor& node) {
    Executor exec(context, scopes);
    node.expr->AcceptVisitor(exec);
    if (context.State.IsThrowing()) return;
    AccessFieldByIndex(*exec.ExpressionValue(), node.pos);
}

void UnaryOpExecutor::VisitIndexAccessor(ast::IndexAccessor& node) {
    Executor exec(context, scopes);
    node.expressionInBrackets->AcceptVisitor(exec);
    if (context.State.IsThrowing()) return;
    auto res = curValue->Subscript(*exec.ExpressionValue());
    if (!res) {
        context.SetThrowingState(runtime::DRuntimeError("Object (of type \"" + curValue->TypeOfValue()->Name() +
                                                        "\") does not support subscripts"),
                                 node.pos);
        return;
    }
    if (res->index()) {
        context.SetThrowingState(get<1>(*res), node.pos);
        return;
    }
    curValue = get<0>(*res);
    curPos = locators::SpanLocator(curPos, node.pos);
}

DISALLOWED_VISIT(Reference)
DISALLOWED_VISIT(XorOperator)
DISALLOWED_VISIT(OrOperator)
DISALLOWED_VISIT(AndOperator)
DISALLOWED_VISIT(BinaryRelation)
DISALLOWED_VISIT(Sum)
DISALLOWED_VISIT(Term)
DISALLOWED_VISIT(Unary)
DISALLOWED_VISIT(UnaryNot)

void UnaryOpExecutor::VisitPrefixOperator(ast::PrefixOperator& node) {
    const char* const 
}

void UnaryOpExecutor::VisitTypecheckOperator(ast::TypecheckOperator& node) {

}

void UnaryOpExecutor::VisitCall(ast::Call& node) {

}

void UnaryOpExecutor::VisitAccessorOperator(ast::AccessorOperator& node) {

}


DISALLOWED_VISIT(PrimaryIdent)
DISALLOWED_VISIT(ParenthesesExpression)
DISALLOWED_VISIT(TupleLiteralElement)
DISALLOWED_VISIT(TupleLiteral)
DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)
DISALLOWED_VISIT(FuncLiteral)
DISALLOWED_VISIT(TokenLiteral)
DISALLOWED_VISIT(ArrayLiteral)
DISALLOWED_VISIT_FULL(Custom, ASTNode)

}  // namespace interp
