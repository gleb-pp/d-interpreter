#include "expressionChecker.h"
#include <memory>
#include <stdexcept>
#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "diagnostics.h"
#include "precomputed.h"
using namespace std;

// may modify the syntax tree
ExpressionChecker::ExpressionChecker(complog::ICompilationLog& log, ValueTimeline& values)
    : log(log), pure(true), values(values) {}
std::variant<std::shared_ptr<runtime::Type>, std::shared_ptr<runtime::RuntimeValue>> ExpressionChecker::Result() const {
    return *res;
}
bool ExpressionChecker::HasResult() const {
    return static_cast<bool>(res);
}
bool ExpressionChecker::Pure() const {
    return pure;
}
std::optional<std::shared_ptr<ast::ASTNode>> ExpressionChecker::Replacement() const {
    return replacement;
}
ValueTimeline& ExpressionChecker::ProgramState() const {
    return values;
}

#define DISALLOWED_VISIT(name) void ExpressionChecker::Visit##name(ast::name& node)\
    { throw std::runtime_error("ExpressionChecker cannot visit ast::"#name); }
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
DISALLOWED_VISIT(IdentMemberAccessor)
DISALLOWED_VISIT(IntLiteralMemberAccessor)
DISALLOWED_VISIT(ParenMemberAccessor)
DISALLOWED_VISIT(IndexAccessor)
DISALLOWED_VISIT(Reference)
DISALLOWED_VISIT(PrefixOperator)
DISALLOWED_VISIT(TypecheckOperator)
DISALLOWED_VISIT(Call)
DISALLOWED_VISIT(AccessorOperator)
DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)

static shared_ptr<ast::Expression> AssertExpression(const shared_ptr<ast::ASTNode>& expr) {
    auto res = dynamic_pointer_cast<ast::Expression>(expr);
    if (!res) throw runtime_error("Expected an expression replacement! ExpressionChecker is implemented incorrectly");
    return res;
}

void ExpressionChecker::VisitLogicalOperator(ExpressionChecker::LogicalOperator kind,
                                             vector<shared_ptr<ast::Expression>>& operands,
                                             const locators::SpanLocator& position) {
    const char* const OPERATOR_NAMES[] = {"xor", "or", "and"};
    const char* OPERATOR_NAME = OPERATOR_NAMES[static_cast<int>(kind)];
    vector<ExpressionChecker> rec;
    size_t valuesknown = 0;
    vector<variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>>> chtypes;
    size_t n = operands.size();
    rec.reserve(n);
    bool errored = false;
    for (size_t i = 0; i < n; i++) {
        auto& ch = operands[i];
        auto& recchecker = rec.emplace_back(log, values);
        ch->AcceptVisitor(recchecker);
        if (!recchecker.HasResult()) {
            errored = true;
            continue;
        }
        chtypes.push_back(recchecker.Result());
        valuesknown += recchecker.Result().index();
        if (recchecker.Replacement()) operands[i] = AssertExpression(*recchecker.Replacement());
        pure = pure && recchecker.Pure();
    }
    if (errored) return;
    if (valuesknown >= 2) {
        vector<shared_ptr<runtime::RuntimeValue>> values;
        values.reserve(valuesknown);
        vector<pair<locators::SpanLocator, shared_ptr<runtime::Type>>> types_positions;
        types_positions.reserve(valuesknown);
        for (size_t i = 0; i < n; i++) {
            if (!chtypes[i].index()) continue;
            values.push_back(get<1>(chtypes[i]));
            types_positions.emplace_back(operands[i]->pos, values.back()->TypeOfValue());
        }
        shared_ptr<runtime::RuntimeValue> res = values.front();
        for (size_t i = 1; i < values.size(); i++) {
            runtime::RuntimeValueResult opresult;
            switch (kind) {
                case LogicalOperator::Xor:
                    opresult = res->BinaryXor(*values[i]);
                    break;
                case LogicalOperator::Or:
                    opresult = res->BinaryOr(*values[i]);
                    break;
                case LogicalOperator::And:
                    opresult = res->BinaryAnd(*values[i]);
                    break;
            }
            if (!opresult) {
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAME, types_positions));
                return;
            }
            if (opresult->index()) {
                log.Log(make_shared<semantic_errors::EvaluationException>(position, get<1>(*opresult).what()));
                return;
            }
            res = get<0>(*opresult);
        }
        {
            size_t j = 0;
            for (size_t i = 0; i < n; i++) {
                if (chtypes[i].index()) continue;
                if (i > j) { chtypes[j] = chtypes[i]; operands[j] = operands[i]; }
                ++j;
            }
        }
        chtypes.insert(chtypes.begin(), res);
        operands.insert(operands.begin(), make_shared<ast::PrecomputedValue>(types_positions.back().first, res));
        n = operands.size();
    }
    vector<pair<locators::SpanLocator, shared_ptr<runtime::Type>>> badTypes;
    for (size_t i = 0; i < n; i++) {
        auto type = chtypes[i].index() ? get<1>(chtypes[i])->TypeOfValue() : get<0>(chtypes[i]);
        if (!type->TypeEq(runtime::BoolType())) badTypes.emplace_back(operands[i]->pos, type);
    }
    if (badTypes.size()) {
        log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAME, badTypes));
        return;
    }
    if (n == 1) {
        replacement = operands[0];
        this->res = chtypes[0];
    } else this->res = make_shared<runtime::BoolType>();
}

void ExpressionChecker::VisitXorOperator(ast::XorOperator& node) {
    VisitLogicalOperator(LogicalOperator::Xor, node.operands, node.pos);
}
void ExpressionChecker::VisitOrOperator(ast::OrOperator& node) {
    VisitLogicalOperator(LogicalOperator::Or, node.operands, node.pos);
}
void ExpressionChecker::VisitAndOperator(ast::AndOperator& node) {
    VisitLogicalOperator(LogicalOperator::And, node.operands, node.pos);
}
void ExpressionChecker::VisitBinaryRelation(ast::BinaryRelation& node) {

}
void ExpressionChecker::VisitSum(ast::Sum& node) {

}
// zero-division is not pure
// division by an int or an unknown is not pure
void ExpressionChecker::VisitTerm(ast::Term& node) {

}
// string.Slice(_, _, 0) is not pure
// string.Slice(_, _, non-0) is pure
// string.Slice(_, _, ?) is not pure
void ExpressionChecker::VisitUnary(ast::Unary& node) {

}
void ExpressionChecker::VisitUnaryNot(ast::UnaryNot& node) {

}
void ExpressionChecker::VisitPrimaryIdent(ast::PrimaryIdent& node) {

}
void ExpressionChecker::VisitParenthesesExpression(ast::ParenthesesExpression& node) {

}
void ExpressionChecker::VisitTupleLiteralElement(ast::TupleLiteralElement& node) {

}
void ExpressionChecker::VisitTupleLiteral(ast::TupleLiteral& node) {

}
void ExpressionChecker::VisitFuncLiteral(ast::FuncLiteral& node) {

}
void ExpressionChecker::VisitTokenLiteral(ast::TokenLiteral& node) {

}
void ExpressionChecker::VisitArrayLiteral(ast::ArrayLiteral& node) {

}
void ExpressionChecker::VisitCustom(ast::ASTNode& node) {

}
