#include "interp/execution.h"
#include <memory>
#include "interp/runtimeContext.h"
#include "runtime/derror.h"
#include "runtime/values.h"
using namespace std;

namespace interp {

Executor::Executor(RuntimeContext& context, const std::shared_ptr<ScopeStack>& scopes)
    : context(context), scopes(scopes) {}

const std::shared_ptr<runtime::RuntimeValue>& Executor::ExpressionValue() const {
    if (!optExprValue) throw runtime_error("Accessed Executor::ExpressionValue(), but it was `nullptr`.");
    return optExprValue;
}

void Executor::VisitBody(ast::Body& node) {
    auto prevScopes = scopes;
    scopes = make_shared<ScopeStack>(scopes);
    for (auto& stmt : node.statements) {
        stmt->AcceptVisitor(*this);
        if (!context.State.IsRunning()) break;
    }
    scopes = prevScopes;
}

void Executor::VisitVarStatement(ast::VarStatement& node) {
    for (auto& def : node.definitions) {
        if (scopes->Lookup(def.first->identifier)) {
            auto span = def.first->span;
            context.SetThrowingState(
                runtime::DRuntimeError("Variable \"" + def.first->identifier + "\" was already declared"),
                                       locators::SpanLocator(node.pos.File(), span.position, span.length));
            return;
        }
        shared_ptr<runtime::RuntimeValue> val;
        if (!def.second)
            val = make_shared<runtime::NoneValue>();
        else {
            auto optVal = ExecuteExpressionInThis(*def.second);
            if (!optVal) return;
            val = *optVal;
        }
        scopes->Declare(make_shared<Variable>(def.first->identifier, val));
    }
}

void Executor::VisitIfStatement(ast::IfStatement& node) {
    auto optcond = ExecuteExpressionInThis(node.condition);
    if (!optcond) return;
    auto condval = dynamic_pointer_cast<runtime::BoolValue>(*optcond);
    if (!condval) {
        context.SetThrowingState(runtime::DRuntimeError(
            "if condition must be a boolean value, but \"" + condval->TypeOfValue()->Name() + "\" was provided"),
                                 node.condition->pos);
        return;
    }
    if (condval->Value()) VisitBody(*node.doIfTrue);
    else if (node.doIfFalse) VisitBody(**node.doIfFalse);
}

void Executor::VisitShortIfStatement(ast::ShortIfStatement& node) {
    auto optcond = ExecuteExpressionInThis(node.condition);
    if (!optcond) return;
    auto condval = dynamic_pointer_cast<runtime::BoolValue>(*optcond);
    if (!condval) {
        context.SetThrowingState(runtime::DRuntimeError(
            "short-if condition must be a boolean value, but \"" + condval->TypeOfValue()->Name() + "\" was provided"),
                                 node.condition->pos);
        return;
    }
    if (condval->Value()) {
        auto prevScopes = scopes;
        scopes = make_shared<ScopeStack>(scopes);
        node.doIfTrue->AcceptVisitor(*this);
        scopes = prevScopes;
    }
}

void Executor::VisitWhileStatement(ast::WhileStatement& node) {
    while (true) {
        auto optcond = ExecuteExpressionInThis(node.condition);
        if (!optcond) return;
        auto condval = dynamic_pointer_cast<runtime::BoolValue>(*optcond);
        if (!condval) {
            context.SetThrowingState(runtime::DRuntimeError(
                "while condition must be a boolean value, but \"" + condval->TypeOfValue()->Name() + "\" was provided"),
                                     node.condition->pos);
            return;
        }
        if (!condval->Value()) break;
        VisitBody(*node.action);
        if (context.State.IsRunning()) continue;
        if (context.State.IsExiting()) context.State = RuntimeState::Running();
        break;
    }
}

void Executor::VisitForStatement(ast::ForStatement& node) {
    // TODO
}

void Executor::VisitLoopStatement(ast::LoopStatement& node) {
    // TODO
}

void Executor::VisitExitStatement(ast::ExitStatement& node) {
    // TODO
}

void Executor::VisitAssignStatement(ast::AssignStatement& node) {
    // TODO
}

void Executor::VisitPrintStatement(ast::PrintStatement& node) {
    // TODO
}

void Executor::VisitReturnStatement(ast::ReturnStatement& node) {
    // TODO
}

void Executor::VisitExpressionStatement(ast::ExpressionStatement& node) {
    // TODO
}

void Executor::VisitCommaExpressions(ast::CommaExpressions& node) {
    // TODO
}

void Executor::VisitCommaIdents(ast::CommaIdents& node) {
    // TODO
}

void Executor::VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) {
    // TODO
}

void Executor::VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) {
    // TODO
}

void Executor::VisitParenMemberAccessor(ast::ParenMemberAccessor& node) {
    // TODO
}

void Executor::VisitIndexAccessor(ast::IndexAccessor& node) {
    // TODO
}

void Executor::VisitReference(ast::Reference& node) {
    // TODO
}

void Executor::VisitXorOperator(ast::XorOperator& node) {
    // TODO
}

void Executor::VisitOrOperator(ast::OrOperator& node) {
    // TODO
}

void Executor::VisitAndOperator(ast::AndOperator& node) {
    // TODO
}

void Executor::VisitBinaryRelation(ast::BinaryRelation& node) {
    // TODO
}

void Executor::VisitSum(ast::Sum& node) {
    // TODO
}

void Executor::VisitTerm(ast::Term& node) {
    // TODO
}

void Executor::VisitUnary(ast::Unary& node) {
    // TODO
}

void Executor::VisitUnaryNot(ast::UnaryNot& node) {
    // TODO
}

void Executor::VisitPrefixOperator(ast::PrefixOperator& node) {
    // TODO
}

void Executor::VisitTypecheckOperator(ast::TypecheckOperator& node) {
    // TODO
}

void Executor::VisitCall(ast::Call& node) {
    // TODO
}

void Executor::VisitAccessorOperator(ast::AccessorOperator& node) {
    // TODO
}

void Executor::VisitPrimaryIdent(ast::PrimaryIdent& node) {
    // TODO
}

void Executor::VisitParenthesesExpression(ast::ParenthesesExpression& node) {
    // TODO
}

void Executor::VisitTupleLiteralElement(ast::TupleLiteralElement& node) {
    // TODO
}

void Executor::VisitTupleLiteral(ast::TupleLiteral& node) {
    // TODO
}

void Executor::VisitShortFuncBody(ast::ShortFuncBody& node) {
    // TODO
}

void Executor::VisitLongFuncBody(ast::LongFuncBody& node) {
    // TODO
}

void Executor::VisitFuncLiteral(ast::FuncLiteral& node) {
    // TODO
}

void Executor::VisitTokenLiteral(ast::TokenLiteral& node) {
    // TODO
}

void Executor::VisitArrayLiteral(ast::ArrayLiteral& node) {
    // TODO
}

void Executor::VisitCustom(ast::ASTNode& node) {
    // TODO
}

}  // namespace interp
