#include "statementChecker.h"
#include "expressionChecker.h"
#include "locators/CodeFile.h"
#include "locators/locator.h"
#include "runtime/types.h"
#include "unaryOpsChecker.h"
#include "diagnostics.h"
#include "syntax.h"
#include "valueTimeline.h"
#include <stdexcept>
using namespace std;

static locators::SpanLocator LocatorFromToken(const Token& tk, const shared_ptr<const locators::CodeFile>& file) {
    return locators::SpanLocator(file, tk.span.position, tk.span.length);
}

StatementChecker::StatementChecker(complog::ICompilationLog& log, ValueTimeline& values, bool inFunction, bool inCycle)
    : log(log),
      pure(true),
      values(values),
      inFunction(inFunction),
      inCycle(inCycle),
      terminationKind(TerminationKind::Errored) {}
bool StatementChecker::Pure() const { return pure; }
std::optional<std::shared_ptr<runtime::Type>> StatementChecker::Returned() const { return returned; }
ValueTimeline& StatementChecker::ProgramState() const { return values; }
StatementChecker::TerminationKind StatementChecker::Terminated() const { return terminationKind; }

#define DISALLOWED_VISIT(name)                                            \
    void StatementChecker::Visit##name(ast::name& node) {                 \
        throw runtime_error("StatementChecker cannot visit ast::" #name); \
    }

static void ReportVariableProblems(complog::ICompilationLog& log, const ScopeStats& stats) {
    for (auto& asg : stats.uselessAssignments)
        log.Log(make_shared<semantic_errors::AssignedValueUnused>(asg.second, asg.first));
    for (auto& var : stats.variablesNeverUsed)
        log.Log(make_shared<semantic_errors::VariableNeverUsed>(var.second, var.first));
}

void StatementChecker::VisitBody(ast::Body& node) {
    values.StartScope();
    size_t n = node.statements.size();
    for (size_t i = 0; i < n; i++) {
        auto stmt = node.statements[i];
        StatementChecker rec(log, values, inFunction, inCycle);
        stmt->AcceptVisitor(rec);
        pure = pure && rec.Pure();
        if (rec.Replacement()) {
            auto& repl = *rec.Replacement();
            if (repl.empty()) {
                node.statements.erase(node.statements.begin() + i);
                --i;
            }
            else {
                node.statements[i] = repl[0];
                node.statements.insert(node.statements.begin() + (i + 1), repl.begin() + 1, repl.end());
                i += repl.size() - 1;
            }
        }
        {
            auto ret = rec.Returned();
            if (ret)
            {
                if (returned) returned = returned.value()->Generalize(**ret);
                else returned = *ret;
            }
        }
        switch (rec.Terminated()) {
            case TerminationKind::ReachedEnd: break;
            case TerminationKind::Returned: case TerminationKind::Exited:
                terminationKind = rec.Terminated();
                if (i < n - 1) {
                    log.Log(make_shared<semantic_errors::CodeUnreachable>(
                        locators::SpanLocator(node.statements[i + 1]->pos, node.statements.back()->pos), true));
                    node.statements.resize(i + 1);
                }
                break;
            case TerminationKind::Errored:
                terminationKind = TerminationKind::Errored;
                values.EndScope();
                return;
        }
    }
    terminationKind = TerminationKind::ReachedEnd;
    ReportVariableProblems(log, values.EndScope());
}

void StatementChecker::VisitVarStatement(ast::VarStatement& node) {
    bool errored = false;
    for (auto& kv : node.definitions) {
        const string& name = kv.first->identifier;
        locators::SpanLocator declarationspan = LocatorFromToken(*kv.first, node.pos.File());
        if (!values.Declare(name, declarationspan)) {
            log.Log(make_shared<semantic_errors::VariableRedefined>(declarationspan, name));
            errored = true;
        }
        if (kv.second) {
            auto& expr = *kv.second;
            ExpressionChecker chk(log, values);
            expr->AcceptVisitor(chk);
            if (!chk.HasResult()) {
                errored = true;
                continue;
            }
            if (chk.Replacement()) expr = chk.AssertReplacementAsExpression();
            pure = pure && chk.Pure();
            values.Assign(name, chk.Result(), kv.second.value()->pos);
        }
    }
    if (errored) return;
    terminationKind = TerminationKind::ReachedEnd;
}

void StatementChecker::VisitIfStatement(ast::IfStatement& node) {
    auto& cond = node.condition;
    ExpressionChecker condchk(log, values);
    if (!condchk.HasResult()) return;
    if (condchk.Replacement()) cond = condchk.AssertReplacementAsExpression();
    pure = condchk.Pure();
    optional<bool> knownCondition;
    {
        auto res = condchk.Result();
        shared_ptr<runtime::Type> type = res.index() ? get<1>(res)->TypeOfValue() : get<0>(res);
        if (!type->TypeEq(runtime::UnknownType()) && !type->TypeEq(runtime::BoolType())) {
            log.Log(make_shared<semantic_errors::ConditionMustBeBoolean>(node.condition->pos, type));
            return;
        }
        if (res.index()) knownCondition = dynamic_pointer_cast<runtime::BoolValue>(get<1>(res))->Value();
    }
    if (knownCondition)
        log.Log(make_shared<semantic_errors::IfConditionAlwaysKnown>(*knownCondition, node.condition->pos));
    
    ValueTimeline elseTL = values;
    StatementChecker truechk(log, values, inFunction, inCycle);
    node.doIfTrue->AcceptVisitor(truechk);
    if (truechk.Replacement()) {
        auto repl = *truechk.Replacement();
        if (repl.size() == 1 && dynamic_cast<ast::Body*>(repl.front().get()))
            node.doIfTrue = dynamic_pointer_cast<ast::Body>(repl.front());
        else
            node.doIfTrue = make_shared<ast::Body>(node.doIfTrue->pos, repl);
    }
    if (node.doIfFalse) {
        auto& doIfFalse = *node.doIfFalse;
        StatementChecker falsechk(log, elseTL, inFunction, inCycle);
        doIfFalse->AcceptVisitor(falsechk);
        if (falsechk.Replacement()) {
            auto repl = *falsechk.Replacement();
            if (repl.size() == 1 && dynamic_cast<ast::Body*>(repl.front().get()))
                doIfFalse = dynamic_pointer_cast<ast::Body>(repl.front());
            else
                doIfFalse = make_shared<ast::Body>(doIfFalse->pos, repl);
        }
        auto trueterm = truechk.Terminated();
        auto falseterm = falsechk.Terminated();
        if (trueterm == TerminationKind::Errored || falseterm == TerminationKind::Errored) return;
        if (knownCondition) {
            replacement.emplace();
            if (!condchk.Pure()) replacement->push_back(make_shared<ast::ExpressionStatement>(cond->pos, cond));
            if (*knownCondition) {
                terminationKind = trueterm;
                pure = pure && truechk.Pure();
                replacement->push_back(node.doIfTrue);
            } else {
                terminationKind = falseterm;
                pure = pure && falsechk.Pure();
                values = elseTL;
                replacement->push_back(doIfFalse);
            }
        } else {
            if (trueterm == TerminationKind::ReachedEnd || falseterm == TerminationKind::ReachedEnd)
                terminationKind = TerminationKind::ReachedEnd;
            else if (trueterm == TerminationKind::Exited || falseterm == TerminationKind::Exited)
                terminationKind = TerminationKind::Exited;
            else
                terminationKind = TerminationKind::Returned;
            pure = pure && truechk.Pure() && falsechk.Pure();
            values.MergeTimelines(elseTL);
        }
    } else {
        auto trueterm = truechk.Terminated();
        if (trueterm == TerminationKind::Errored) return;
        if (knownCondition) {
            replacement.emplace();
            if (!condchk.Pure()) replacement->push_back(make_shared<ast::ExpressionStatement>(cond->pos, cond));
            if (*knownCondition) {
                terminationKind = trueterm;
                pure = pure && truechk.Pure();
                replacement->push_back(node.doIfTrue);
            } else {
                terminationKind = TerminationKind::ReachedEnd;
                values = elseTL;
            }
        } else {
            terminationKind = TerminationKind::ReachedEnd;
            pure = pure && truechk.Pure();
            values.MergeTimelines(elseTL);
        }
    }
}

void StatementChecker::VisitShortIfStatement(ast::ShortIfStatement& node) {
    auto doiftrue = make_shared<ast::Body>(node.doIfTrue->pos, vector<shared_ptr<ast::Statement>>{node.doIfTrue});
    shared_ptr<ast::IfStatement> equiv =
        make_shared<ast::IfStatement>(node.pos, node.condition, doiftrue, optional<shared_ptr<ast::Body>>());
    VisitIfStatement(*equiv);
    if (!replacement) replacement = vector<shared_ptr<ast::Statement>>{equiv};
}

void StatementChecker::VisitWhileStatement(ast::WhileStatement& node) {
    // first evaluation:
    auto tl = values;

}

/*
void StatementChecker::VisitForStatement(ast::ForStatement& node) 
void StatementChecker::VisitLoopStatement(ast::LoopStatement& node) 
void StatementChecker::VisitExitStatement(ast::ExitStatement& node) 
*/

void StatementChecker::VisitAssignStatement(ast::AssignStatement& node) {
    pure = false;
    ExpressionChecker valuechk(log, values);
    node.src->AcceptVisitor(valuechk);
    if (!valuechk.HasResult()) return;
    if (valuechk.Replacement()) node.src = valuechk.AssertReplacementAsExpression();
    auto srcval = valuechk.Result();
    auto& ref = node.dest;
    size_t n = ref->accessorChain.size();
    if (!n) {
        if (!values.Assign(ref->baseIdent->identifier, srcval, node.pos)) {
            auto span = ref->baseIdent->span;
            log.Log(make_shared<semantic_errors::VariableNotDefined>(
                locators::SpanLocator(node.pos.File(), span.position, span.length), ref->baseIdent->identifier));
            return;
        }
        terminationKind = TerminationKind::ReachedEnd;
        return;
    }

    runtime::TypeOrValue cur;
    locators::SpanLocator curloc = LocatorFromToken(*ref->baseIdent, node.pos.File());
    {
        auto optval = values.LookupVariable(ref->baseIdent->identifier);
        if (!optval) {
            log.Log(make_shared<semantic_errors::VariableNotDefined>(curloc, ref->baseIdent->identifier));
            return;
        }
        cur = *optval;
    }
    for (size_t i = 0; i + 1 < n; i++) {
        UnaryOpChecker chk(log, values, cur, curloc);
        auto acc = ref->accessorChain[i];
        acc->AcceptVisitor(chk);
        if (!chk.HasResult()) return;
        cur = chk.Result();
        curloc = locators::SpanLocator(curloc, acc->pos);
    }
    auto acc = ref->accessorChain.back();
    auto type = cur.index() ? get<1>(cur)->TypeOfValue() : get<0>(cur);
    auto lastloc = acc->pos;

    {  // subscript
        auto index = dynamic_cast<ast::IndexAccessor*>(acc.get());
        if (index) {
            if (!type->TypeEq(runtime::UnknownType()) && !type->TypeEq(runtime::ArrayType())) {
                log.Log(make_shared<semantic_errors::SubscriptAssignmentOnlyInArrays>(lastloc, type));
                return;
            }
            ExpressionChecker chk(log, values);
            index->expressionInBrackets->AcceptVisitor(chk);
            if (!chk.HasResult()) return;
            if (chk.Replacement()) index->expressionInBrackets = chk.AssertReplacementAsExpression();
            auto ind = chk.Result();
            auto indtype = ind.index() ? get<1>(ind)->TypeOfValue() : get<0>(ind);
            if (!indtype->TypeEq(runtime::IntegerType()) && !indtype->TypeEq(runtime::UnknownType())) {
                log.Log(make_shared<semantic_errors::BadSubscriptIndexType>(lastloc, indtype));
                return;
            }
            if (cur.index() && ind.index() && srcval.index()) {
                const BigInt& indexvalue = dynamic_cast<runtime::IntegerValue&>(*get<1>(ind)).Value();
                dynamic_cast<runtime::ArrayValue&>(*get<1>(cur)).AssignItem(indexvalue, get<1>(srcval));
            }
            terminationKind = TerminationKind::ReachedEnd;
            return;
        }
    }

    // if not subscript, then field assignments
    if (!type->TypeEq(runtime::UnknownType()) && !type->TypeEq(runtime::TupleType())) {
        log.Log(make_shared<semantic_errors::FieldsOnlyAssignableInTuples>(lastloc, type));
        return;
    }
    auto destAsTuple = cur.index() ? dynamic_cast<runtime::TupleValue*>(get<1>(cur).get()) : nullptr;

    {  // named
        auto named = dynamic_cast<ast::IdentMemberAccessor*>(acc.get());
        if (named) {
            auto name = named->name->identifier;
            if (destAsTuple && srcval.index()) {
                if (!destAsTuple->AssignNamedField(name, get<1>(srcval))) {
                    log.Log(make_shared<semantic_errors::CannotAssignNamedFieldInTuple>(lastloc, name));
                    return;
                }
            }
            terminationKind = TerminationKind::ReachedEnd;
            return;
        }
    }
    {  // indexed
        optional<BigInt> indexvalue;
        auto exprIndex = dynamic_cast<ast::ParenMemberAccessor*>(acc.get());
        if (exprIndex) {
            if (!type->TypeEq(runtime::UnknownType()) && !type->TypeEq(runtime::ArrayType())) {
                log.Log(make_shared<semantic_errors::SubscriptAssignmentOnlyInArrays>(lastloc, type));
                return;
            }
            ExpressionChecker chk(log, values);
            exprIndex->expr->AcceptVisitor(chk);
            if (!chk.HasResult()) return;
            if (chk.Replacement()) exprIndex->expr = chk.AssertReplacementAsExpression();
            auto ind = chk.Result();
            auto indtype = ind.index() ? get<1>(ind)->TypeOfValue() : get<0>(ind);
            if (!indtype->TypeEq(runtime::IntegerType()) && !indtype->TypeEq(runtime::UnknownType())) {
                log.Log(make_shared<semantic_errors::BadSubscriptIndexType>(lastloc, indtype));
                return;
            }
            if (ind.index()) indexvalue = dynamic_cast<runtime::IntegerValue&>(*get<1>(ind)).Value();
        }
        auto constIndex = dynamic_cast<ast::IntLiteralMemberAccessor*>(acc.get());
        if (constIndex) indexvalue = constIndex->index->value;
#ifdef DINTERP_DEBUG
        if (!constIndex && !exprIndex) throw runtime_error("unhandled accessor assignment");
#endif
        if (indexvalue && cur.index() && srcval.index()) {
            if (!dynamic_cast<runtime::TupleValue&>(*get<1>(cur)).AssignIndexedField(*indexvalue, get<1>(srcval))) {
                log.Log(make_shared<semantic_errors::CannotAssignIndexedFieldInTuple>(lastloc, indexvalue->ToString()));
                return;
            }
        }
        terminationKind = TerminationKind::ReachedEnd;
        return;
    }
}
        /*
void StatementChecker::VisitPrintStatement(ast::PrintStatement& node) 
void StatementChecker::VisitReturnStatement(ast::ReturnStatement& node) 
void StatementChecker::VisitExpressionStatement(ast::ExpressionStatement& node) 
        */

DISALLOWED_VISIT(CommaExpressions)
DISALLOWED_VISIT(CommaIdents)
DISALLOWED_VISIT(IdentMemberAccessor)
DISALLOWED_VISIT(IntLiteralMemberAccessor)
DISALLOWED_VISIT(ParenMemberAccessor)
DISALLOWED_VISIT(IndexAccessor)
DISALLOWED_VISIT(Reference)
DISALLOWED_VISIT(XorOperator)
DISALLOWED_VISIT(OrOperator)
DISALLOWED_VISIT(AndOperator)
DISALLOWED_VISIT(BinaryRelation)
DISALLOWED_VISIT(Sum)
DISALLOWED_VISIT(Term)
DISALLOWED_VISIT(Unary)
DISALLOWED_VISIT(UnaryNot)
DISALLOWED_VISIT(PrefixOperator)
DISALLOWED_VISIT(TypecheckOperator)
DISALLOWED_VISIT(Call)
DISALLOWED_VISIT(AccessorOperator)
DISALLOWED_VISIT(PrimaryIdent)
DISALLOWED_VISIT(ParenthesesExpression)
DISALLOWED_VISIT(TupleLiteralElement)
DISALLOWED_VISIT(TupleLiteral)
DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)
DISALLOWED_VISIT(FuncLiteral)
DISALLOWED_VISIT(TokenLiteral)
DISALLOWED_VISIT(ArrayLiteral)

void StatementChecker::VisitCustom(ast::ASTNode& node) {
    throw runtime_error("StatementChecker cannot visit a Custom node");
}
