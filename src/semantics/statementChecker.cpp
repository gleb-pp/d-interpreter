#include "statementChecker.h"
#include "expressionChecker.h"
#include "diagnostics.h"
#include "valueTimeline.h"
#include <stdexcept>
using namespace std;

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
        switch (rec.Terminated()) {
            case TerminationKind::ReachedEnd: break;
            case TerminationKind::Returned:
                returned = rec.Returned();
            case TerminationKind::Exited:
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
        auto decl_span = kv.first->span;
        locators::SpanLocator declarationspan(node.pos.File(), decl_span.position, decl_span.length);
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

void StatementChecker::VisitIfStatement(ast::IfStatement& node) 
void StatementChecker::VisitShortIfStatement(ast::ShortIfStatement& node) 
void StatementChecker::VisitWhileStatement(ast::WhileStatement& node) 
void StatementChecker::VisitForStatement(ast::ForStatement& node) 
void StatementChecker::VisitLoopStatement(ast::LoopStatement& node) 
void StatementChecker::VisitExitStatement(ast::ExitStatement& node) 
void StatementChecker::VisitAssignStatement(ast::AssignStatement& node) {

}
void StatementChecker::VisitPrintStatement(ast::PrintStatement& node) 
void StatementChecker::VisitReturnStatement(ast::ReturnStatement& node) 
void StatementChecker::VisitExpressionStatement(ast::ExpressionStatement& node) 

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
