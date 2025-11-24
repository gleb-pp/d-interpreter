#include "dinterp/interp/execution.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include "dinterp/interp/closure.h"
#include "dinterp/interp/runtimeContext.h"
#include "dinterp/interp/unaryOpExec.h"
#include "dinterp/locators/locator.h"
#include "dinterp/runtime/derror.h"
#include "dinterp/runtime/values.h"
#include "dinterp/syntax.h"
#include "dinterp/syntaxext/precomputed.h"
using namespace std;

namespace interp {

Executor::Executor(RuntimeContext& context, const std::shared_ptr<ScopeStack>& scopes)
    : context(context), scopes(scopes) {}

const std::shared_ptr<runtime::RuntimeValue>& Executor::ExpressionValue() const {
    if (!optExprValue) throw runtime_error("Accessed Executor::ExpressionValue(), but it was `nullptr`.");
    return optExprValue;
}

optional<shared_ptr<runtime::RuntimeValue>> Executor::ExecuteExpressionInThis(const shared_ptr<ast::Expression>& expr) {
    expr->AcceptVisitor(*this);
    if (!context.State.IsRunning()) {
        optExprValue.reset();
        return {};
    }
    auto val = ExpressionValue();
    optExprValue.reset();
    return val;
}

void Executor::ExecuteOperators(const std::vector<std::shared_ptr<ast::Expression>>& operands,
                                const std::vector<OperatorKind>& ops) {
    const char* const OPNAMES[] = {"+", "-", "*", "/"};
    locators::SpanLocator cur = operands[0]->pos;
    shared_ptr<runtime::RuntimeValue> val;
    {
        auto optVal = ExecuteExpressionInThis(operands[0]);
        if (!optVal) return;
        val = *optVal;
    }
    size_t n = ops.size();
    for (size_t i = 0; i < n; i++) {
        auto optRHS = ExecuteExpressionInThis(operands[i + 1]);
        if (!optRHS) return;
        cur = locators::SpanLocator(cur, operands[i + 1]->pos);
        runtime::RuntimeValueResult res;
        switch (ops[i]) {
            case OperatorKind::Plus:
                res = val->BinaryPlus(**optRHS);
                break;
            case OperatorKind::Minus:
                res = val->BinaryMinus(**optRHS);
                break;
            case OperatorKind::Times:
                res = val->BinaryMul(**optRHS);
                break;
            case OperatorKind::Divide:
                res = val->BinaryDiv(**optRHS);
                break;
        }
        if (!res) {
            context.SetThrowingState(
                runtime::DRuntimeError(string("Operator \"") + OPNAMES[static_cast<int>(ops[i])] +
                                       "\" is not supported between \"" + val->TypeOfValue()->Name() + "\" and \"" +
                                       optRHS.value()->TypeOfValue()->Name() + "\""),
                cur);
            return;
        }
        if (res->index()) {
            context.SetThrowingState(get<1>(*res), cur);
            return;
        }
        val = get<0>(*res);
    }
    optExprValue = val;
}

void Executor::ExecuteLogicalOperators(LogicalOperatorKind kind,
                                       const std::vector<std::shared_ptr<ast::Expression>>& operands) {
    const char* const OPNAMES[] = {"and", "or", "xor"};
    optional<bool> stopValue;
    switch (kind) {
        case LogicalOperatorKind::And:
            stopValue = false;
            break;
        case LogicalOperatorKind::Or:
            stopValue = true;
            break;
        case LogicalOperatorKind::Xor:
            break;
    }
    shared_ptr<runtime::BoolValue> val;
    auto curpos = operands[0]->pos;
    {
        auto optVal = ExecuteExpressionInThis(operands[0]);
        if (!optVal) return;
        val = dynamic_pointer_cast<runtime::BoolValue>(*optVal);
        if (!val) {
            context.SetThrowingState(runtime::DRuntimeError(string("Operator \"") + OPNAMES[static_cast<int>(kind)] +
                                                            "\" expects boolean operands, but got \"" +
                                                            optVal.value()->TypeOfValue()->Name() + "\""),
                                     curpos);
            return;
        }
    }
    size_t n = operands.size();
    for (size_t i = 1; i < n && (!stopValue || *stopValue != val->Value()); i++) {
        curpos = locators::SpanLocator(curpos, operands[i]->pos);
        auto optRHS = ExecuteExpressionInThis(operands[i]);
        if (!optRHS) return;
        runtime::RuntimeValueResult res;
        switch (kind) {
            case LogicalOperatorKind::And:
                res = val->BinaryAnd(**optRHS);
                break;
            case LogicalOperatorKind::Or:
                res = val->BinaryOr(**optRHS);
                break;
            case LogicalOperatorKind::Xor:
                res = val->BinaryXor(**optRHS);
                break;
        }
        if (!res) {
            context.SetThrowingState(runtime::DRuntimeError(string("Operator \"") + OPNAMES[static_cast<int>(kind)] +
                                                            "\" is not applicable to \"" + val->TypeOfValue()->Name() +
                                                            "\" and \"" + optRHS.value()->TypeOfValue()->Name() + "\""),
                                     curpos);
            return;
        }
        if (res->index()) {
            context.SetThrowingState(get<1>(*res), curpos);
            return;
        }
        val = dynamic_pointer_cast<runtime::BoolValue>(get<0>(*res));
    }
    optExprValue = val;
}

#define DISALLOWED_VISIT(name) \
    void Executor::Visit##name(ast::name&) { throw runtime_error("Executor cannot visit " #name); }

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
        context.SetThrowingState(runtime::DRuntimeError("if condition must be a boolean value, but \"" +
                                                        optcond.value()->TypeOfValue()->Name() + "\" was provided"),
                                 node.condition->pos);
        return;
    }
    if (condval->Value())
        VisitBody(*node.doIfTrue);
    else if (node.doIfFalse)
        VisitBody(**node.doIfFalse);
}

void Executor::VisitShortIfStatement(ast::ShortIfStatement& node) {
    auto optcond = ExecuteExpressionInThis(node.condition);
    if (!optcond) return;
    auto condval = dynamic_pointer_cast<runtime::BoolValue>(*optcond);
    if (!condval) {
        context.SetThrowingState(runtime::DRuntimeError("short-if condition must be a boolean value, but \"" +
                                                        optcond.value()->TypeOfValue()->Name() + "\" was provided"),
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
            context.SetThrowingState(runtime::DRuntimeError("while condition must be a boolean value, but \"" +
                                                            optcond.value()->TypeOfValue()->Name() + "\" was provided"),
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
    optional<shared_ptr<Variable>> cyclevar;
    if (node.optVariableName)
        cyclevar = make_shared<Variable>(node.optVariableName.value()->identifier, make_shared<runtime::NoneValue>());

    shared_ptr<runtime::RuntimeValue> startOrList;
    {
        auto optStartOrList = ExecuteExpressionInThis(node.startOrList);
        if (!optStartOrList) return;
        startOrList = *optStartOrList;
    }
    optional<variant<pair<BigInt, BigInt>, vector<shared_ptr<runtime::RuntimeValue>>, size_t>> range;
    if (node.end) {
        auto intstart = dynamic_pointer_cast<runtime::IntegerValue>(startOrList);
        if (!intstart) {
            context.SetThrowingState(
                runtime::DRuntimeError("Starting bound was of type \"" + startOrList->TypeOfValue()->Name() +
                                       "\", expected an integer"),
                node.startOrList->pos);
            return;
        }
        auto end = ExecuteExpressionInThis(*node.end);
        if (!end) return;
        auto intend = dynamic_pointer_cast<runtime::IntegerValue>(*end);
        if (!intend) {
            context.SetThrowingState(
                runtime::DRuntimeError("Ending bound was of type \"" + end.value()->TypeOfValue()->Name() +
                                       "\", expected an integer"),
                node.end.value()->pos);
            return;
        }
        range.emplace(make_pair(intstart->Value(), intend->Value()));
    } else {
        auto arrayval = dynamic_pointer_cast<runtime::ArrayValue>(startOrList);
        if (arrayval) {
            if (cyclevar) {
                range.emplace(vector<shared_ptr<runtime::RuntimeValue>>(arrayval->Value.size()));
                ranges::transform(
                    arrayval->Value, get<1>(*range).begin(),
                    [](const pair<const BigInt, shared_ptr<runtime::RuntimeValue>>& p) { return p.second; });
            } else
                range.emplace(arrayval->Value.size());
        } else {
            auto tupleval = dynamic_pointer_cast<runtime::TupleValue>(startOrList);
            if (tupleval) {
                if (cyclevar)
                    range.emplace(tupleval->Values());
                else
                    range.emplace(tupleval->Values().size());
            } else {
                context.SetThrowingState(
                    runtime::DRuntimeError("Expected an iterable type (array or tuple), but got \"" +
                                           startOrList->TypeOfValue()->Name()),
                    node.startOrList->pos);
                return;
            }
        }
    }

    auto prevScopes = scopes;
    if (cyclevar) {
        scopes = make_shared<ScopeStack>(scopes);
        scopes->Declare(*cyclevar);
    }
    switch (range->index()) {
        case 0: {  // range from BigInt to BigInt
            auto& [start, end] = get<0>(*range);
            bool decrement = start > end;
            auto cur = start;
            while (true) {
                if (cyclevar) cyclevar.value()->Assign(make_shared<runtime::IntegerValue>(cur));
                VisitBody(*node.action);
                if (!context.State.IsRunning()) {
                    if (context.State.IsExiting()) context.State = RuntimeState::Running();
                    break;
                }
                if (cur == end) break;
                if (decrement)
                    --cur;
                else
                    ++cur;
            }
            break;
        }
        case 1: {  // using a cycle variable, iterating over a collection
            auto& items = get<1>(*range);
            for (auto& item : items) {
                cyclevar.value()->Assign(item);
                VisitBody(*node.action);
                if (!context.State.IsRunning()) {
                    if (context.State.IsExiting()) context.State = RuntimeState::Running();
                    break;
                }
            }
            break;
        }
        case 2: {  // iterating several times without a variable
            size_t n = get<2>(*range);
            for (size_t i = 0; i < n; ++i) {
                VisitBody(*node.action);
                if (!context.State.IsRunning()) {
                    if (context.State.IsExiting()) context.State = RuntimeState::Running();
                    break;
                }
            }
            break;
        }
    }
    scopes = prevScopes;
}

void Executor::VisitLoopStatement(ast::LoopStatement& node) {
    while (true) {
        VisitBody(*node.body);
        if (!context.State.IsRunning()) {
            if (context.State.IsExiting()) context.State = RuntimeState::Running();
            break;
        }
    }
}

void Executor::VisitExitStatement(ast::ExitStatement&) { context.State = RuntimeState::Exiting(); }

void Executor::VisitAssignStatement(ast::AssignStatement& node) {
    shared_ptr<runtime::RuntimeValue> val;
    {
        auto optVal = ExecuteExpressionInThis(node.src);
        if (!optVal) return;
        val = *optVal;
    }
    auto optvariable = scopes->Lookup(node.dest->baseIdent->identifier);
    auto _span = node.dest->baseIdent->span;
    locators::SpanLocator curpos(node.pos.File(), _span.position, _span.length);
    if (!optvariable) {
        context.SetThrowingState(
            runtime::DRuntimeError("Variable not declared: \"" + node.dest->baseIdent->identifier + "\""), curpos);
        return;
    }
    if (node.dest->accessorChain.empty()) {
        optvariable.value()->Assign(val);
        return;
    }
    size_t n = node.dest->accessorChain.size() - 1;
    auto curobj = optvariable.value()->Content();
    if (n) {
        UnaryOpExecutor exec(context, scopes, curobj, curpos);
        for (size_t i = 0; i < n; ++i) {
            node.dest->accessorChain[i]->AcceptVisitor(exec);
            if (context.State.IsThrowing()) return;
        }
        curobj = exec.Value();
        curpos = exec.Position();
    }
    auto lastAccessor = node.dest->accessorChain.back();
    auto indexAcc = dynamic_pointer_cast<ast::IndexAccessor>(lastAccessor);
    if (indexAcc) {
        auto arr = dynamic_pointer_cast<runtime::ArrayValue>(curobj);
        if (!arr) {
            context.SetThrowingState(runtime::DRuntimeError("Can only assign by subscript to arrays, tried with \"" +
                                                            curobj->TypeOfValue()->Name() + "\""),
                                     curpos);
            return;
        }
        auto indObj = ExecuteExpressionInThis(indexAcc->expressionInBrackets);
        if (!indObj) return;
        auto intSubscript = dynamic_pointer_cast<runtime::IntegerValue>(*indObj);
        if (!intSubscript) {
            context.SetThrowingState(runtime::DRuntimeError("Subscript must be an integer, but it was \"" +
                                                            indObj.value()->TypeOfValue()->Name() + "\""),
                                     curpos);
            return;
        }
        arr->AssignItem(intSubscript->Value(), val);
        return;
    }

    auto tuple = dynamic_pointer_cast<runtime::TupleValue>(curobj);
    if (!tuple) {
        context.SetThrowingState(runtime::DRuntimeError("Can only assign by field to tuples, tried with \"" +
                                                        curobj->TypeOfValue()->Name() + "\""),
                                 curpos);
        return;
    }
    auto namedAcc = dynamic_pointer_cast<ast::IdentMemberAccessor>(lastAccessor);
    if (namedAcc) {
        if (tuple->AssignNamedField(namedAcc->name->identifier, val)) return;
        context.SetThrowingState(runtime::DRuntimeError("No field named \"" + namedAcc->name->identifier + "\""),
                                 namedAcc->pos);
        return;
    }
    BigInt index;
    auto paren = dynamic_pointer_cast<ast::ParenMemberAccessor>(lastAccessor);
    if (paren) {
        auto indObj = ExecuteExpressionInThis(paren->expr);
        if (!indObj) return;
        auto intSubscript = dynamic_pointer_cast<runtime::IntegerValue>(*indObj);
        if (!intSubscript) {
            context.SetThrowingState(runtime::DRuntimeError("Field index must be an integer, but it was \"" +
                                                            indObj.value()->TypeOfValue()->Name() + "\""),
                                     curpos);
            return;
        }
        index = intSubscript->Value();
    } else
        index = dynamic_cast<ast::IntLiteralMemberAccessor&>(*lastAccessor).index->value;
    if (tuple->AssignIndexedField(index, val)) return;
    context.SetThrowingState(runtime::DRuntimeError("Field index out of range: " + index.ToString()),
                             lastAccessor->pos);
}

void Executor::VisitPrintStatement(ast::PrintStatement& node) {
    for (auto& expr : node.expressions) {
        auto val = ExecuteExpressionInThis(expr);
        if (!val) return;
        val.value()->PrintSelf(*context.Output);
    }
    context.Output->flush();
}

void Executor::VisitReturnStatement(ast::ReturnStatement& node) {
    shared_ptr<runtime::RuntimeValue> ret;
    if (!node.returnValue)
        ret = make_shared<runtime::NoneValue>();
    else {
        auto opt = ExecuteExpressionInThis(*node.returnValue);
        if (!opt) return;
        ret = *opt;
    }
    context.State = RuntimeState::Returning(ret);
}

void Executor::VisitExpressionStatement(ast::ExpressionStatement& node) {
    node.expr->AcceptVisitor(*this);
    optExprValue.reset();
}

DISALLOWED_VISIT(CommaExpressions)
DISALLOWED_VISIT(CommaIdents)
DISALLOWED_VISIT(IdentMemberAccessor)
DISALLOWED_VISIT(IntLiteralMemberAccessor)
DISALLOWED_VISIT(ParenMemberAccessor)
DISALLOWED_VISIT(IndexAccessor)
DISALLOWED_VISIT(Reference)

void Executor::VisitXorOperator(ast::XorOperator& node) {
    ExecuteLogicalOperators(LogicalOperatorKind::Xor, node.operands);
}

void Executor::VisitOrOperator(ast::OrOperator& node) {
    ExecuteLogicalOperators(LogicalOperatorKind::Or, node.operands);
}

void Executor::VisitAndOperator(ast::AndOperator& node) {
    ExecuteLogicalOperators(LogicalOperatorKind::And, node.operands);
}

void Executor::VisitBinaryRelation(ast::BinaryRelation& node) {
    shared_ptr<runtime::RuntimeValue> lhs;
    {
        auto optLHS = ExecuteExpressionInThis(node.operands[0]);
        if (!optLHS) return;
        lhs = *optLHS;
    }
    size_t n = node.operators.size();
    for (size_t i = 0; i < n; i++) {
        auto op = node.operators[i];
        auto optRHS = ExecuteExpressionInThis(node.operands[i + 1]);
        if (!optRHS) return;
        auto comp = lhs->BinaryComparison(**optRHS);
        if (!comp) {
            context.SetThrowingState(
                runtime::DRuntimeError("Objects of types \"" + lhs->TypeOfValue()->Name() + "\" and \"" +
                                       optRHS.value()->TypeOfValue()->Name() + "\" are incomparable"),
                locators::SpanLocator(node.operands[i]->pos, node.operands[i + 1]->pos));
            return;
        }
        bool result = false;
        switch (op) {
            case ast::BinaryRelationOperator::Less:
                result = *comp < 0;
                break;
            case ast::BinaryRelationOperator::LessEq:
                result = *comp <= 0;
                break;
            case ast::BinaryRelationOperator::Greater:
                result = *comp > 0;
                break;
            case ast::BinaryRelationOperator::GreaterEq:
                result = *comp >= 0;
                break;
            case ast::BinaryRelationOperator::Equal:
                result = *comp == 0;
                break;
            case ast::BinaryRelationOperator::NotEqual:
                result = *comp != 0;
                break;
        }
        if (!result) {
            optExprValue = make_shared<runtime::BoolValue>(false);
            return;
        }
        lhs = *optRHS;
    }
    optExprValue = make_shared<runtime::BoolValue>(true);
}

void Executor::VisitSum(ast::Sum& node) {
    vector<OperatorKind> ops(node.operators.size());
    ranges::transform(node.operators, ops.begin(), [](ast::Sum::SumOperator op) {
        return op == ast::Sum::SumOperator::Plus ? OperatorKind::Plus : OperatorKind::Minus;
    });
    ExecuteOperators(node.terms, ops);
}

void Executor::VisitTerm(ast::Term& node) {
    vector<OperatorKind> ops(node.operators.size());
    ranges::transform(node.operators, ops.begin(), [](ast::Term::TermOperator op) {
        return op == ast::Term::TermOperator::Times ? OperatorKind::Times : OperatorKind::Divide;
    });
    ExecuteOperators(node.unaries, ops);
}

void Executor::VisitUnary(ast::Unary& node) {
    shared_ptr<runtime::RuntimeValue> val;
    {
        auto opt = ExecuteExpressionInThis(node.expr);
        if (!opt) return;
        val = *opt;
    }
    auto preiter = node.prefixOps.rbegin();
    auto preend = node.prefixOps.rend();
    auto postiter = node.postfixOps.begin();
    auto postend = node.postfixOps.end();
    UnaryOpExecutor unaryexec(context, scopes, val, node.expr->pos);
    while (true) {
        bool executePrefix = true;
        if (preiter != preend) {
            if (postiter != postend) executePrefix = (*preiter)->precedence() < (*postiter)->precedence();
        } else if (postiter != postend)
            executePrefix = false;
        else
            break;
        shared_ptr<ast::ASTNode> operation =
            executePrefix ? static_cast<shared_ptr<ast::ASTNode>>(*(preiter++)) : *(postiter++);
        operation->AcceptVisitor(unaryexec);
        if (context.State.IsThrowing()) return;
    }
    optExprValue = unaryexec.Value();
}

void Executor::VisitUnaryNot(ast::UnaryNot& node) {
    auto opt = ExecuteExpressionInThis(node.nested);
    if (!opt) return;
    auto res = opt.value()->UnaryNot();
    if (!res) {
        context.SetThrowingState(
            runtime::DRuntimeError("The unary not operator does not support an operand of type \"" +
                                   opt.value()->TypeOfValue()->Name() + "\""),
            node.nested->pos);
        return;
    }
    if (res->index()) {
        context.SetThrowingState(get<1>(*res), node.pos);
        return;
    }
    optExprValue = get<0>(*res);
}

DISALLOWED_VISIT(PrefixOperator)
DISALLOWED_VISIT(TypecheckOperator)
DISALLOWED_VISIT(Call)
DISALLOWED_VISIT(AccessorOperator)

void Executor::VisitPrimaryIdent(ast::PrimaryIdent& node) {
    auto var = scopes->Lookup(node.name->identifier);
    if (!var) {
        context.SetThrowingState(
            runtime::DRuntimeError("Referencing an undeclared variable: \"" + node.name->identifier + "\""), node.pos);
        return;
    }
    optExprValue = var.value()->Content();
}

void Executor::VisitParenthesesExpression(ast::ParenthesesExpression& node) { node.expr->AcceptVisitor(*this); }

DISALLOWED_VISIT(TupleLiteralElement)

void Executor::VisitTupleLiteral(ast::TupleLiteral& node) {
    size_t n = node.elements.size();
    set<string> seenNames;
    vector<pair<optional<string>, shared_ptr<runtime::RuntimeValue>>> vals;
    vals.reserve(n);
    for (auto& elem : node.elements) {
        optional<string> name;
        if (elem->ident) {
            if (!seenNames.insert(elem->ident.value()->identifier).second) {
                auto span = elem->ident.value()->span;
                context.SetThrowingState(runtime::DRuntimeError("Field name duplicated"),
                                         locators::SpanLocator(node.pos.File(), span.position, span.length));
                return;
            }
            name.emplace(elem->ident.value()->identifier);
        }
        auto opt = ExecuteExpressionInThis(elem->expression);
        if (!opt) return;
        vals.emplace_back(name, *opt);
    }
    optExprValue = make_shared<runtime::TupleValue>(vals);
}

DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)
DISALLOWED_VISIT(FuncLiteral)  // must be replaced with a ClosureDefinition by the semantic analyzer

void Executor::VisitTokenLiteral(ast::TokenLiteral& node) {
    switch (node.kind) {
        case ast::TokenLiteral::TokenLiteralKind::False:
            optExprValue = make_shared<runtime::BoolValue>(false);
            break;
        case ast::TokenLiteral::TokenLiteralKind::True:
            optExprValue = make_shared<runtime::BoolValue>(true);
            break;
        case ast::TokenLiteral::TokenLiteralKind::String:
            optExprValue = make_shared<runtime::StringValue>(dynamic_cast<StringLiteral&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::Int:
            optExprValue = make_shared<runtime::IntegerValue>(dynamic_cast<IntegerToken&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::Real:
            optExprValue = make_shared<runtime::RealValue>(dynamic_cast<RealToken&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::None:
            optExprValue = make_shared<runtime::NoneValue>();
            break;
    }
}

void Executor::VisitArrayLiteral(ast::ArrayLiteral& node) {
    size_t n = node.items.size();
    vector<shared_ptr<runtime::RuntimeValue>> vals;
    vals.reserve(n);
    for (auto& elem : node.items) {
        auto opt = ExecuteExpressionInThis(elem);
        if (!opt) return;
        vals.emplace_back(*opt);
    }
    optExprValue = make_shared<runtime::ArrayValue>(vals);
}

void Executor::VisitCustom(ast::ASTNode& node) {
    auto precomp = dynamic_cast<ast::PrecomputedValue*>(&node);
    if (precomp) {
        optExprValue = precomp->Value;
        return;
    }
    auto closdef = dynamic_cast<ast::ClosureDefinition*>(&node);
    if (!closdef) throw runtime_error("Custom node not recognized by Executor");
    optExprValue = make_shared<runtime::Closure>(scopes, *closdef);
}

}  // namespace interp
