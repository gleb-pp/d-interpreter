#include "interp/unaryOpExec.h"

#include <memory>
#include <sstream>

#include "interp/execution.h"
#include "interp/userCallable.h"
#include "locators/locator.h"
#include "runtime/derror.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "syntax.h"
using namespace std;

namespace interp {

UnaryOpExecutor::UnaryOpExecutor(RuntimeContext& context, const std::shared_ptr<ScopeStack>& scopes,
                                 const std::shared_ptr<runtime::RuntimeValue> curValue,
                                 const locators::SpanLocator& curPos)
    : context(context), scopes(scopes), curValue(curValue), curPos(curPos) {}

const std::shared_ptr<runtime::RuntimeValue>& UnaryOpExecutor::Value() const { return curValue; }

const locators::SpanLocator& UnaryOpExecutor::Position() const { return curPos; }

#define DISALLOWED_VISIT_FULL(name, classname)                              \
    void UnaryOpExecutor::Visit##name(ast::classname&) {                    \
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
    const char* const OPNAMES[] = {"unary +", "unary -"};
    runtime::RuntimeValueResult res =
        node.kind == ast::PrefixOperator::PrefixOperatorKind::Plus ? curValue->UnaryPlus() : curValue->UnaryMinus();
    if (!res) {
        context.SetThrowingState(
            runtime::DRuntimeError("Object (of type \"" + curValue->TypeOfValue()->Name() +
                                   "\") does not support the " + OPNAMES[static_cast<int>(node.kind)] + " operator"),
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

void UnaryOpExecutor::VisitTypecheckOperator(ast::TypecheckOperator& node) {
    unique_ptr<runtime::Type> type;
    switch (node.typeId) {
        case ast::TypeId::Int:
            type = make_unique<runtime::IntegerType>();
            break;
        case ast::TypeId::Real:
            type = make_unique<runtime::RealType>();
            break;
        case ast::TypeId::String:
            type = make_unique<runtime::StringType>();
            break;
        case ast::TypeId::Bool:
            type = make_unique<runtime::BoolType>();
            break;
        case ast::TypeId::None:
            type = make_unique<runtime::NoneType>();
            break;
        case ast::TypeId::Func:
            type = make_unique<runtime::FuncType>();
            break;
        case ast::TypeId::Tuple:
            type = make_unique<runtime::TupleType>();
            break;
        case ast::TypeId::List:
            type = make_unique<runtime::ArrayType>();
            break;
    }
    curValue = make_shared<runtime::BoolValue>(curValue->TypeOfValue()->TypeEq(*type));
    curPos = locators::SpanLocator(curPos, node.pos);
}

void UnaryOpExecutor::VisitCall(ast::Call& node) {
    size_t n = node.args.size();
    vector<shared_ptr<runtime::RuntimeValue>> args;
    args.reserve(n);
    for (auto& arg : node.args) {
        Executor exec(context, scopes);
        arg->AcceptVisitor(exec);
        if (context.State.IsThrowing()) return;
        args.push_back(exec.ExpressionValue());
    }
    curPos = locators::SpanLocator(curPos, node.pos);
    auto userfunc = dynamic_cast<interp::UserCallable*>(curValue.get());
    if (userfunc) {
        if (!context.Stack.Push(curPos)) {
            context.SetThrowingState(runtime::DRuntimeError("Stack overflow!"), curPos);
            return;
        }
        auto ret = userfunc->UserCall(context, args);
        context.Stack.Pop();
        if (context.State.IsThrowing()) return;
#ifdef DINTERP_DEBUG
        if (!ret) throw runtime_error("User-callable function returned nullptr");
#endif
        curValue = ret;
        return;
    }
    auto func = dynamic_cast<runtime::FuncValue*>(curValue.get());
    if (func) {
        auto res = func->Call(args);
        if (res) {
            if (res->index()) {
                context.SetThrowingState(get<1>(*res), curPos);
                return;
            }
            curValue = get<0>(*res);
            return;
        }
    }
    context.SetThrowingState(
        runtime::DRuntimeError("Cannot call this object of type \"" + curValue->TypeOfValue()->Name() + "\""), curPos);
}

void UnaryOpExecutor::VisitAccessorOperator(ast::AccessorOperator& node) { node.accessor->AcceptVisitor(*this); }

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
