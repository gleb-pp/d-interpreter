#include "expressionChecker.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "diagnostics.h"
#include "precomputed.h"
#include "syntax.h"
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
std::shared_ptr<ast::Expression> ExpressionChecker::AssertReplacementAsExpression() const {
    auto res = dynamic_pointer_cast<ast::Expression>(*replacement);
    if (!res) throw runtime_error("Expected an expression replacement! ExpressionChecker is implemented incorrectly");
    return res;
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
        if (recchecker.Replacement()) operands[i] = recchecker.AssertReplacementAsExpression();
        pure = pure && recchecker.Pure();
    }
    if (errored) return;
    if (pure && valuesknown >= 2) {
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
            chtypes.resize(j);
            operands.resize(j);
        }
        chtypes.insert(chtypes.begin(), res);
        operands.insert(operands.begin(), make_shared<ast::PrecomputedValue>(types_positions.back().first, res));
        n = operands.size();
    }
    vector<pair<locators::SpanLocator, shared_ptr<runtime::Type>>> badTypes;
    for (size_t i = 0; i < n; i++) {
        auto type = chtypes[i].index() ? get<1>(chtypes[i])->TypeOfValue() : get<0>(chtypes[i]);
        if (!type->TypeEq(runtime::BoolType()) && !type->TypeEq(runtime::UnknownType()))
            badTypes.emplace_back(operands[i]->pos, type);
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
    const char* const OPERATOR_NAMES[] = {
        "<", "<=", ">", ">=", "=", "/="
    };
    auto& operands = node.operands;
    auto& operators = node.operators;
    vector<optional<shared_ptr<runtime::RuntimeValue>>> optValues;
    vector<shared_ptr<runtime::Type>> types;
    size_t n = operands.size();
    optValues.reserve(n);
    types.reserve(n);
    bool errored = false;
    vector<bool> operand_pure;
    operand_pure.reserve(n);
    for (size_t i = 0; i < n; i++) {
        ExpressionChecker recur(log, values);
        auto& ch = operands[i];
        ch->AcceptVisitor(recur);
        if (!recur.HasResult()) {
            errored = true;
            continue;
        }
        operand_pure.push_back(recur.Pure());
        pure = pure && recur.Pure();
        if (recur.Replacement()) ch = recur.AssertReplacementAsExpression();
        const auto& res = recur.Result();
        if (res.index())
            types.push_back(optValues.emplace_back(get<1>(res)).value()->TypeOfValue());
        else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
        }
    }
    if (errored) return;
    for (size_t i = 1; i < n; i++) {
        shared_ptr<runtime::Type>& a = types[i - 1], & b = types[i];
        auto op = operators[i - 1];
        bool ok;
        if (op == ast::BinaryRelationOperator::Equal || op == ast::BinaryRelationOperator::NotEqual)
            ok = a->BinaryEq(*b);
        else
            ok = a->BinaryOrdering(*b);
        if (!ok) {
            errored = true;
            semantic_errors::VectorOfSpanTypes bad_pos{make_pair(operands[i - 1]->pos, a),
                                                       make_pair(operands[i]->pos, b)};
            log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad_pos));
        }
    }
    if (errored) return;
    while (n > 1 && optValues[0] && optValues[1] && operand_pure[0] && (operand_pure[1] || n > 2)) {
        auto res = optValues[0].value()->BinaryComparison(**optValues[1]);
        auto op = operators[0];
        if (!res) {
            semantic_errors::VectorOfSpanTypes bad_pos{make_pair(operands[0]->pos, optValues[0].value()->TypeOfValue()),
                                                       make_pair(operands[1]->pos, optValues[1].value()->TypeOfValue())};
            log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad_pos));
            return;
        }
        bool cmp;
        switch (op) {
            case ast::BinaryRelationOperator::Less: cmp = *res < 0; break;
            case ast::BinaryRelationOperator::LessEq: cmp = *res <= 0; break;
            case ast::BinaryRelationOperator::Greater: cmp = *res > 0; break;
            case ast::BinaryRelationOperator::GreaterEq: cmp = *res >= 0; break;
            case ast::BinaryRelationOperator::Equal: cmp = *res == 0; break;
            case ast::BinaryRelationOperator::NotEqual: cmp = *res != 0; break;
        }
        if (cmp) {
            --n;
            operands.erase(operands.begin());
            operators.erase(operators.begin());
            operand_pure.erase(operand_pure.begin());
            optValues.erase(optValues.begin());
            types.erase(types.begin());
            continue;
        }
        if (!pure) break;
        auto _false = make_shared<runtime::BoolValue>(false);
        replacement = make_shared<ast::PrecomputedValue>(node.pos, _false);
        this->res = _false;
        return;
    }
    if (n == 1) {
        auto _true = make_shared<runtime::BoolValue>(true);
        replacement = make_shared<ast::PrecomputedValue>(node.pos, _true);
        this->res = _true;
        return;
    }
    this->res = make_shared<runtime::BoolType>();
}

void ExpressionChecker::VisitSum(ast::Sum& node) {
    const char* const OPERATOR_NAMES[] = {"+", "-"};
    auto& operands = node.terms;
    auto& operators = node.operators;
    size_t n = operands.size();
    vector<optional<shared_ptr<runtime::RuntimeValue>>> optValues;
    optValues.reserve(n);
    vector<shared_ptr<runtime::Type>> types;
    types.reserve(n);
    bool errored = false;
    vector<bool> operand_pure;
    operand_pure.reserve(n);
    size_t knownpurevalues = 0;
    bool numeric = false;
    for (size_t i = 0; i < n; i++) {
        ExpressionChecker rec(log, values);
        auto& ch = operands[i];
        ch->AcceptVisitor(rec);
        if (!rec.HasResult()) {
            errored = true;
            continue;
        }
        operand_pure.push_back(rec.Pure());
        pure = pure && rec.Pure();
        if (rec.Replacement()) ch = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
        if (res.index()) {
            types.push_back(optValues.emplace_back(get<1>(res)).value()->TypeOfValue());
            knownpurevalues += rec.Pure();
        }
        else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
        }
        numeric = numeric || (types.back()->TypeEq(runtime::IntegerType()) || types.back()->TypeEq(runtime::RealType()));
    }
    if (errored) return;

    if (knownpurevalues > 1 && numeric) {
        shared_ptr<runtime::RuntimeValue> curvalue; optional<locators::SpanLocator> loc;
        vector<bool> deletion(n);
        for (size_t i = 0; i < n; i++) {
            if (!optValues[i] || !operand_pure[i]) continue;
            deletion[i] = true;
            if (!loc) {
                bool neg = i && operators[i - 1] == ast::Sum::SumOperator::Minus;
                curvalue = *optValues[i];
                loc = operands[i]->pos;
                if (neg) {
                    auto negated = curvalue->UnaryMinus();
                    if (!negated) {
                        semantic_errors::VectorOfSpanTypes bad{{*loc, curvalue->TypeOfValue()}};
                        log.Log(make_shared<semantic_errors::OperatorNotApplicable>("-", bad));
                        return;
                    }
                    // no need to check for exceptions
                    curvalue = get<0>(*negated);
                }
                continue;
            }
            ast::Sum::SumOperator op = operators[i - 1];
            auto& vali = **optValues[i];
            const auto& loci = operands[i]->pos;
            runtime::RuntimeValueResult add =
                (op == ast::Sum::SumOperator::Plus) ? curvalue->BinaryPlus(vali) : curvalue->BinaryMinus(vali);
            if (!add) {
                semantic_errors::VectorOfSpanTypes badtypes{{*loc, curvalue->TypeOfValue()}, {loci, vali.TypeOfValue()}};
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], badtypes));
                return;
            }
            auto mergedloc = locators::SpanLocator(*loc, loci);
            if (add->index()) {
                log.Log(make_shared<semantic_errors::EvaluationException>(mergedloc, get<1>(*add).what()));
                return;
            }
            curvalue = get<0>(*add);
            loc = mergedloc;
        }
        size_t j = 0;
        for (size_t i = 0; i < n; i++) {
            if (deletion[i]) continue;
            if (i > j) {
                operands[j] = operands[i];
                operators[j - 1] = operators[i - 1];
                types[j] = types[i];
                optValues[j] = optValues[i];
                operand_pure[j] = operand_pure[i];
            }
            ++j;
        }
        operands.resize(j);
        operators.resize(j - 1);
        types.resize(j);
        optValues.resize(j);
        operand_pure.resize(j);
        n = j + 1;
        operators.insert(operators.begin(), ast::Sum::SumOperator::Plus);
        operands.insert(operands.begin(), make_shared<ast::PrecomputedValue>(*loc, curvalue));
        types.insert(types.begin(), curvalue->TypeOfValue());
        optValues.insert(optValues.begin(), curvalue);
        operand_pure.insert(operand_pure.begin(), true);
    } else {  // merge sequential
        for (size_t i = 1; i < n; i++) {
            if (!(operand_pure[i - 1] && operand_pure[i] && optValues[i - 1] && optValues[i])) continue;
            auto leftop = i > 1 ? operators[i - 2] : ast::Sum::SumOperator::Plus;
            auto op = leftop == operators[i - 1] ? ast::Sum::SumOperator::Plus : ast::Sum::SumOperator::Minus;
            auto& lhs = *optValues[i - 1].value();
            auto& rhs = *optValues[i].value();
            auto val = op == ast::Sum::SumOperator::Plus ? lhs.BinaryPlus(rhs) : lhs.BinaryMinus(rhs);
            if (!val) {
                semantic_errors::VectorOfSpanTypes bad{{operands[i - 1]->pos, lhs.TypeOfValue()},
                                                       {operands[i]->pos, rhs.TypeOfValue()}};
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
                return;
            }
            auto mergedloc = locators::SpanLocator(operands[i - 1]->pos, operands[i]->pos);
            if (val->index()) {
                log.Log(make_shared<semantic_errors::EvaluationException>(mergedloc, get<1>(*val).what()));
                return;
            }
            auto value = get<0>(*val);
            operators.erase(operators.begin() + (i - 1));
            operands.erase(operands.begin() + i);
            types.erase(types.begin() + i);
            optValues.erase(optValues.begin() + i);
            operand_pure.erase(operand_pure.begin() + i);
            operands[i - 1] = make_shared<ast::PrecomputedValue>(mergedloc, value);
            optValues[i - 1] = value;
            types[i - 1] = value->TypeOfValue();
            --n;
            --i;
        }
    }

    shared_ptr<runtime::Type> curtype = types.front();
    {
        locators::SpanLocator curloc = operands.front()->pos;
        for (size_t i = 1; i < n; i++) {
            const shared_ptr<runtime::Type>& b = types[i];
            auto op = operators[i - 1];
            locators::SpanLocator loc = operands[i]->pos;
            optional<shared_ptr<runtime::Type>> restype =
                (op == ast::Sum::SumOperator::Plus) ? curtype->BinaryPlus(*b) : curtype->BinaryMinus(*b);
            if (!restype) {
                semantic_errors::VectorOfSpanTypes badtypes{{curloc, curtype}, {loc, b}};
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], badtypes));
                return;
            }
            curloc = locators::SpanLocator(curloc, loc);
            curtype = *restype;
        }
    }
    this->res = curtype;
}

// zero-division is not pure
// division by an int or an unknown is not pure
void ExpressionChecker::VisitTerm(ast::Term& node) {
    const char* const OPERATOR_NAMES[] = {"*", "/"};
    auto& operands = node.unaries;
    auto& operators = node.operators;
    size_t n = operands.size();
    vector<optional<shared_ptr<runtime::RuntimeValue>>> optValues;
    optValues.reserve(n);
    vector<shared_ptr<runtime::Type>> types;
    types.reserve(n);
    bool errored = false;
    vector<bool> operand_pure;
    bool allknown = true;
    for (size_t i = 0; i < n; i++) {
        ExpressionChecker rec(log, values);
        auto& ch = operands[i];
        ch->AcceptVisitor(rec);
        if (!rec.HasResult()) {
            errored = true;
            continue;
        }
        pure = pure && rec.Pure();
        if (rec.Replacement()) ch = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
        if (res.index()) {
            types.push_back(optValues.emplace_back(get<1>(res)).value()->TypeOfValue());
            allknown = allknown && rec.Pure();
        }
        else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
            allknown = false;
        }
    }
    if (errored) return;

    if (allknown) {
        locators::SpanLocator loc = operands[0]->pos;
        vector<shared_ptr<runtime::RuntimeValue>> values(n);
        std::ranges::transform(optValues, values.begin(), [](const optional<shared_ptr<runtime::RuntimeValue>>& val) { return *val; });
        auto cur = values[0];
        for (size_t i = 1; i < n; i++) {
            const locators::SpanLocator& newloc = operands[i]->pos;
            auto& newval = values[i];
            auto op = operators[i];
            runtime::RuntimeValueResult res = op == ast::Term::TermOperator::Times ? cur->BinaryMul(*newval) : cur->BinaryDiv(*newval);
            if (!res) {
                semantic_errors::VectorOfSpanTypes bad{{loc, cur->TypeOfValue()}, {newloc, newval->TypeOfValue()}};
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
                return;
            }
            auto mergedloc = locators::SpanLocator(loc, newloc);
            if (res->index()) {
                log.Log(make_shared<semantic_errors::EvaluationException>(mergedloc, get<1>(*res).what()));
                return;
            }
            loc = mergedloc;
            cur = get<0>(*res);
        }
        this->res = cur;
        this->replacement = make_shared<ast::PrecomputedValue>(node.pos, cur);
        return;
    }

    auto curtype = types[0];
    auto loc = operands[0]->pos;
    for (int i = 1; i < n; i++) {
        auto op = operators[i - 1];
        auto newloc = operands[i]->pos;
        auto b = types[i];
        auto optnewtype = op == ast::Term::TermOperator::Times ? curtype->BinaryMul(*b) : curtype->BinaryDiv(*b);
        if (!optnewtype) {
            semantic_errors::VectorOfSpanTypes bad{{loc, curtype}, {newloc, b}};
            log.Log(make_shared<semantic_errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
            return;
        }
        curtype = *optnewtype;
        loc = locators::SpanLocator(loc, newloc);
        if (op == ast::Term::TermOperator::Divide && (b->TypeEq(runtime::IntegerType()) || b->TypeEq(runtime::UnknownType()))) {
            if (!optValues[i]) {
                pure = false;
                continue;
            }
            auto bval = dynamic_cast<runtime::IntegerValue&>(*b);
            if (!bval.Value()) log.Log(make_shared<semantic_errors::IntegerZeroDivisionWarning>(newloc));
        }
    }
    this->res = curtype;
}

// string.Slice(_, _, 0) is not pure
// string.Slice(_, _, non-0) is pure
// string.Slice(_, _, ?) is not pure

static optional<variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>>> ProcessUnaryValue(
    bool& unite, bool& pure, shared_ptr<runtime::RuntimeValue>& cur, locators::SpanLocator& loc, ast::ASTNode& unaryOp,
    const locators::SpanLocator& opLoc) {
    ast::IdentMemberAccessor* nameField = dynamic_cast<ast::IdentMemberAccessor*>(&unaryOp);
    if (nameField) {
        
    }
}

void ExpressionChecker::VisitUnary(ast::Unary& node) {
    ExpressionChecker rec(log, values);
    node.expr->AcceptVisitor(rec);
    if (!rec.HasResult()) return;
    if (rec.Replacement()) node.expr = rec.AssertReplacementAsExpression();
    pure = rec.Pure();
    if (node.prefixOps.empty() && node.postfixOps.empty()) {
        replacement = rec.replacement;
        res = rec.res;
        return;
    }
    locators::SpanLocator loc = node.expr->pos;
    auto val = rec.Result();
    if (pure)
        while (val.index()) {
            if (node.postfixOps.size()) {
                auto post = node.postfixOps.front();
                
            }
        }
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
