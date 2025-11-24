#include "semantic/expressionChecker.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include "lexer.h"
#include "locators/locator.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "semantic/diagnostics.h"
#include "semantic/statementChecker.h"
#include "semantic/unaryOpsChecker.h"
#include "syntax.h"
#include "syntaxext/precomputed.h"
using namespace std;

namespace semantic {

// may modify the syntax tree
ExpressionChecker::ExpressionChecker(complog::ICompilationLog& log, ValueTimeline& values)
    : log(log), pure(true), values(values) {}
runtime::TypeOrValue ExpressionChecker::Result() const { return *res; }
bool ExpressionChecker::HasResult() const { return static_cast<bool>(res); }
bool ExpressionChecker::Pure() const { return pure; }
std::optional<std::shared_ptr<ast::ASTNode>> ExpressionChecker::Replacement() const { return replacement; }
std::shared_ptr<ast::Expression> ExpressionChecker::AssertReplacementAsExpression() const {
    auto res = dynamic_pointer_cast<ast::Expression>(*replacement);
    if (!res) throw runtime_error("Expected an expression replacement! ExpressionChecker is implemented incorrectly");
    return res;
}
ValueTimeline& ExpressionChecker::ProgramState() const { return values; }

#define DISALLOWED_VISIT(name)                                                  \
    void ExpressionChecker::Visit##name([[maybe_unused]] ast::name& node) {     \
        throw std::runtime_error("ExpressionChecker cannot visit ast::" #name); \
    }
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
DISALLOWED_VISIT(TupleLiteralElement)

static inline bool isUnknown(const runtime::TypeOrValue& res) {
    return !res.index() && get<0>(res)->TypeEq(runtime::UnknownType());
}

void ExpressionChecker::VisitAndOrOperator(bool isOr, vector<shared_ptr<ast::Expression>>& operands,
                                           const locators::SpanLocator& position) {
    const char* const OPERATOR_NAME = isOr ? "or" : "and";
    const bool IDEMPOTENT_VAL = !isOr;
    size_t n = operands.size();
    vector<ExpressionChecker> recs;
    recs.reserve(n);
    vector<ValueTimeline> tls;
    tls.reserve(n);
    bool errored = false;
    for (size_t i = 0; i < n; i++) {
        auto& op = operands[i];
        auto& tl = tls.emplace_back(recs.empty() ? values : tls.back());
        auto& rec = recs.emplace_back(log, tl);
        op->AcceptVisitor(rec);
        if (!rec.HasResult()) errored = true;
        if (rec.Replacement()) op = rec.AssertReplacementAsExpression();
        if (isUnknown(rec.Result())) pure = false;
    }
    if (errored) return;

    auto cur = recs[0].Result();
    pure = recs[0].Pure();
    locators::SpanLocator loc(operands[0]->pos);
    size_t cut_first = 0;
    if (cur.index()) {
        auto val = get<1>(cur);
        if (!val->TypeOfValue()->TypeEq(runtime::BoolType())) {
            errors::VectorOfSpanTypes bad{{loc, val->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, bad));
            return;
        }
        if (dynamic_cast<runtime::BoolValue&>(*val).Value() == IDEMPOTENT_VAL) {
            if (pure) cut_first = 1;
        } else {
            values = tls[0];
            this->res = val;
            if (pure)
                replacement = make_shared<ast::PrecomputedValue>(position, val);
            else
                operands.resize(1);
            if (1 < n) {
                log.Log(make_shared<errors::CodeUnreachable>(
                    locators::SpanLocator(operands[1]->pos, operands.back()->pos), true));
            }
            return;
        }
    }
    for (size_t i = 1; i < n; i++) {
        auto& rec = recs[i];
        auto ch = rec.Result();
        auto newloc = operands[i]->pos;
        pure = pure && rec.Pure();
        if (cur.index() && ch.index()) {
            auto optres = isOr ? get<1>(cur)->BinaryOr(*get<1>(ch)) : get<1>(cur)->BinaryAnd(*get<1>(ch));
            if (!optres) {
                errors::VectorOfSpanTypes bad{{loc, get<1>(cur)->TypeOfValue()}, {newloc, get<1>(ch)->TypeOfValue()}};
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, bad));
                return;
            }
            auto mergedloc = locators::SpanLocator(loc, newloc);
            if (optres->index()) {
                log.Log(make_shared<errors::EvaluationException>(mergedloc, get<1>(*optres).what()));
                return;
            }
            auto val = get<0>(*optres);
            cur = val;
            loc = mergedloc;
            if (dynamic_cast<runtime::BoolValue&>(*val).Value() == IDEMPOTENT_VAL) {
                if (pure) cut_first = i + 1;
            } else {
                values = tls[i];
                this->res = val;
                if (pure)
                    replacement = make_shared<ast::PrecomputedValue>(position, val);
                else
                    operands.resize(i + 1);
                if (i + 1 < n) {
                    log.Log(make_shared<errors::CodeUnreachable>(
                        locators::SpanLocator(operands[i + 1]->pos, operands.back()->pos), true));
                }
                return;
            }
            continue;
        }
        auto curtype = cur.index() ? get<1>(cur)->TypeOfValue() : get<0>(cur);
        auto newtype = ch.index() ? get<1>(ch)->TypeOfValue() : get<0>(ch);
        auto optres = curtype->BinaryLogical(*newtype);
        if (!optres) {
            errors::VectorOfSpanTypes bad{{loc, get<1>(cur)->TypeOfValue()}, {newloc, get<1>(ch)->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, bad));
            return;
        }
        auto& restype = *optres;
        cur = restype;
        loc = locators::SpanLocator(loc, newloc);
    }
    values = tls.back();  // THIS IS ONLY CORRECT BECAUSE VARIABLES CANNOT CHANGE (can only become unknown)
    operands.erase(operands.begin(), operands.begin() + cut_first);
    if (operands.empty()) {
        auto id = make_shared<runtime::BoolValue>(IDEMPOTENT_VAL);
        res = id;
        replacement = make_shared<ast::PrecomputedValue>(position, id);
        return;
    }
    res = cur;
    if (res->index() && pure) replacement = make_shared<ast::PrecomputedValue>(position, get<1>(cur));
}

void ExpressionChecker::VisitXorOperator(ast::XorOperator& node) {
    const char* OPERATOR_NAME = "xor";
    vector<ExpressionChecker> rec;
    size_t valuesknown = 0;
    vector<variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>>> chtypes;
    size_t n = node.operands.size();
    rec.reserve(n);
    bool errored = false;
    for (size_t i = 0; i < n; i++) {
        auto& ch = node.operands[i];
        auto& recchecker = rec.emplace_back(log, values);
        ch->AcceptVisitor(recchecker);
        if (!recchecker.HasResult()) {
            errored = true;
            continue;
        }
        chtypes.push_back(recchecker.Result());
        valuesknown += recchecker.Result().index();
        if (recchecker.Replacement()) node.operands[i] = recchecker.AssertReplacementAsExpression();
        pure = pure && recchecker.Pure() && !isUnknown(recchecker.Result());
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
            types_positions.emplace_back(node.operands[i]->pos, values.back()->TypeOfValue());
        }
        shared_ptr<runtime::RuntimeValue> res = values.front();
        for (size_t i = 1; i < values.size(); i++) {
            runtime::RuntimeValueResult opresult;
            opresult = res->BinaryXor(*values[i]);
            if (!opresult) {
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, types_positions));
                return;
            }
            if (opresult->index()) {
                log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*opresult).what()));
                return;
            }
            res = get<0>(*opresult);
        }
        {
            size_t j = 0;
            for (size_t i = 0; i < n; i++) {
                if (chtypes[i].index()) continue;
                if (i > j) {
                    chtypes[j] = chtypes[i];
                    node.operands[j] = node.operands[i];
                }
                ++j;
            }
            chtypes.resize(j);
            node.operands.resize(j);
        }
        chtypes.insert(chtypes.begin(), res);
        node.operands.insert(node.operands.begin(),
                             make_shared<ast::PrecomputedValue>(types_positions.back().first, res));
        n = node.operands.size();
    }
    vector<pair<locators::SpanLocator, shared_ptr<runtime::Type>>> badTypes;
    for (size_t i = 0; i < n; i++) {
        auto type = chtypes[i].index() ? get<1>(chtypes[i])->TypeOfValue() : get<0>(chtypes[i]);
        if (!type->TypeEq(runtime::BoolType()) && !type->TypeEq(runtime::UnknownType()))
            badTypes.emplace_back(node.operands[i]->pos, type);
    }
    if (badTypes.size()) {
        log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, badTypes));
        return;
    }
    if (n == 1) {
        replacement = node.operands[0];
        this->res = chtypes[0];
    } else
        this->res = make_shared<runtime::BoolType>();
}

void ExpressionChecker::VisitOrOperator(ast::OrOperator& node) { VisitAndOrOperator(true, node.operands, node.pos); }

void ExpressionChecker::VisitAndOperator(ast::AndOperator& node) { VisitAndOrOperator(false, node.operands, node.pos); }

void ExpressionChecker::VisitBinaryRelation(ast::BinaryRelation& node) {
    const char* const OPERATOR_NAMES[] = {"<", "<=", ">", ">=", "=", "/="};
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
        pure = pure && recur.Pure() && !isUnknown(recur.Result());
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
        shared_ptr<runtime::Type>&a = types[i - 1], &b = types[i];
        auto op = operators[i - 1];
        bool ok;
        if (op == ast::BinaryRelationOperator::Equal || op == ast::BinaryRelationOperator::NotEqual)
            ok = a->BinaryEq(*b);
        else
            ok = a->BinaryOrdering(*b);
        if (!ok) {
            errored = true;
            errors::VectorOfSpanTypes bad_pos{make_pair(operands[i - 1]->pos, a), make_pair(operands[i]->pos, b)};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad_pos));
        }
    }
    if (errored) return;
    while (n > 1 && optValues[0] && optValues[1] && operand_pure[0] && (operand_pure[1] || n > 2)) {
        auto res = optValues[0].value()->BinaryComparison(**optValues[1]);
        auto op = operators[0];
        if (!res) {
            errors::VectorOfSpanTypes bad_pos{make_pair(operands[0]->pos, optValues[0].value()->TypeOfValue()),
                                              make_pair(operands[1]->pos, optValues[1].value()->TypeOfValue())};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad_pos));
            return;
        }
        bool cmp;
        switch (op) {
            case ast::BinaryRelationOperator::Less:
                cmp = *res < 0;
                break;
            case ast::BinaryRelationOperator::LessEq:
                cmp = *res <= 0;
                break;
            case ast::BinaryRelationOperator::Greater:
                cmp = *res > 0;
                break;
            case ast::BinaryRelationOperator::GreaterEq:
                cmp = *res >= 0;
                break;
            case ast::BinaryRelationOperator::Equal:
                cmp = *res == 0;
                break;
            case ast::BinaryRelationOperator::NotEqual:
                cmp = *res != 0;
                break;
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
        if (n > 2)
            log.Log(make_shared<errors::CodeUnreachable>(locators::SpanLocator(operands[2]->pos, operands.back()->pos),
                                                         true));
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
        pure = pure && rec.Pure() && !isUnknown(rec.Result());
        if (rec.Replacement()) ch = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
        if (res.index()) {
            types.push_back(optValues.emplace_back(get<1>(res)).value()->TypeOfValue());
            knownpurevalues += rec.Pure();
        } else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
        }
        numeric =
            numeric || (types.back()->TypeEq(runtime::IntegerType()) || types.back()->TypeEq(runtime::RealType()));
    }
    if (errored) return;

    if (knownpurevalues > 1 && numeric) {
        shared_ptr<runtime::RuntimeValue> curvalue;
        optional<locators::SpanLocator> loc;
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
                        errors::VectorOfSpanTypes bad{{*loc, curvalue->TypeOfValue()}};
                        log.Log(make_shared<errors::OperatorNotApplicable>("-", bad));
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
                errors::VectorOfSpanTypes badtypes{{*loc, curvalue->TypeOfValue()}, {loci, vali.TypeOfValue()}};
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], badtypes));
                return;
            }
            auto mergedloc = locators::SpanLocator(*loc, loci);
            if (add->index()) {
                log.Log(make_shared<errors::EvaluationException>(mergedloc, get<1>(*add).what()));
                return;
            }
            curvalue = get<0>(*add);
            loc = mergedloc;
        }
        size_t j = 0;
        operators.insert(operators.begin(), ast::Sum::SumOperator::Plus);
        for (size_t i = 0; i < n; i++) {
            if (deletion[i]) continue;
            if (i > j) {
                operands[j] = operands[i];
                operators[j] = operators[i];
                types[j] = types[i];
                optValues[j] = optValues[i];
                operand_pure[j] = operand_pure[i];
            }
            ++j;
        }
        operands.resize(j);
        operators.resize(j);
        types.resize(j);
        optValues.resize(j);
        operand_pure.resize(j);
        n = j + 1;
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
                errors::VectorOfSpanTypes bad{{operands[i - 1]->pos, lhs.TypeOfValue()},
                                              {operands[i]->pos, rhs.TypeOfValue()}};
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
                return;
            }
            auto mergedloc = locators::SpanLocator(operands[i - 1]->pos, operands[i]->pos);
            if (val->index()) {
                log.Log(make_shared<errors::EvaluationException>(mergedloc, get<1>(*val).what()));
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

    if (n == 1 && pure && optValues.front()) {
        replacement = make_shared<ast::PrecomputedValue>(node.pos, *optValues.front());
        this->res = *optValues.front();
        return;
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
                errors::VectorOfSpanTypes badtypes{{curloc, curtype}, {loc, b}};
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], badtypes));
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
        pure = pure && rec.Pure() && !isUnknown(rec.Result());
        if (rec.Replacement()) ch = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
        if (res.index()) {
            types.push_back(optValues.emplace_back(get<1>(res)).value()->TypeOfValue());
            allknown = allknown && rec.Pure();
        } else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
            allknown = false;
        }
    }
    if (errored) return;

    if (allknown) {
        locators::SpanLocator loc = operands[0]->pos;
        vector<shared_ptr<runtime::RuntimeValue>> values(n);
        std::ranges::transform(optValues, values.begin(),
                               [](const optional<shared_ptr<runtime::RuntimeValue>>& val) { return *val; });
        auto cur = values[0];
        for (size_t i = 1; i < n; i++) {
            const locators::SpanLocator& newloc = operands[i]->pos;
            auto& newval = values[i];
            auto op = operators[i - 1];
            runtime::RuntimeValueResult res =
                op == ast::Term::TermOperator::Times ? cur->BinaryMul(*newval) : cur->BinaryDiv(*newval);
            if (!res) {
                errors::VectorOfSpanTypes bad{{loc, cur->TypeOfValue()}, {newloc, newval->TypeOfValue()}};
                log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
                return;
            }
            auto mergedloc = locators::SpanLocator(loc, newloc);
            if (res->index()) {
                log.Log(make_shared<errors::EvaluationException>(mergedloc, get<1>(*res).what()));
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
    for (size_t i = 1; i < n; i++) {
        auto op = operators[i - 1];
        auto newloc = operands[i]->pos;
        auto b = types[i];
        auto optnewtype = op == ast::Term::TermOperator::Times ? curtype->BinaryMul(*b) : curtype->BinaryDiv(*b);
        if (!optnewtype) {
            errors::VectorOfSpanTypes bad{{loc, curtype}, {newloc, b}};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAMES[static_cast<int>(op)], bad));
            return;
        }
        auto prevtype = curtype;
        curtype = *optnewtype;
        loc = locators::SpanLocator(loc, newloc);
        if (op == ast::Term::TermOperator::Divide &&
            (b->TypeEq(runtime::IntegerType()) || b->TypeEq(runtime::UnknownType()))) {
            if (prevtype->TypeEq(runtime::RealType())) continue;
            if (!optValues[i]) {
                pure = false;
                continue;
            }
            auto bval = dynamic_cast<runtime::IntegerValue&>(**optValues[i]);
            if (!bval.Value()) log.Log(make_shared<errors::IntegerZeroDivisionWarning>(newloc));
        }
    }
    this->res = curtype;
}

// string.Slice(_, _, 0) is not pure
// string.Slice(_, _, non-0) is pure
// string.Slice(_, _, ?) is not pure

void ExpressionChecker::VisitUnary(ast::Unary& node) {
    ExpressionChecker rec(log, values);
    node.expr->AcceptVisitor(rec);
    if (!rec.HasResult()) return;
    if (rec.Replacement()) node.expr = rec.AssertReplacementAsExpression();
    pure = rec.Pure() && !isUnknown(rec.Result());
    if (node.prefixOps.empty() && node.postfixOps.empty()) {
        replacement = rec.replacement ? rec.replacement : node.expr;
        res = rec.res;
        return;
    }
    locators::SpanLocator loc = node.expr->pos;
    auto val = rec.Result();

    bool precomp = pure && val.index();
    size_t npost = node.postfixOps.size(), npre = node.prefixOps.size();
    size_t ipost = 0, ipre = 0;
    while (ipost < npost || ipre < npre) {
        bool doPostfix = ipre == npre || (ipost < npost && node.prefixOps[npre - ipre - 1]->precedence() <
                                                               node.postfixOps[ipost]->precedence());
        if (doPostfix) {
            UnaryOpChecker chk(log, values, val, loc);
            node.postfixOps[ipost]->AcceptVisitor(chk);
            if (!chk.HasResult()) return;
            pure = pure && chk.Pure();
            val = chk.Result();
            auto iloc = node.postfixOps[ipost]->pos;
            loc = locators::SpanLocator(loc, iloc);
            precomp = precomp && pure && val.index() && !get<1>(val)->TypeOfValue()->Mutable();
            if (precomp) {
                node.postfixOps.erase(node.postfixOps.begin());
                npost--;
                node.expr = make_shared<ast::PrecomputedValue>(loc, get<1>(val));
            } else
                ++ipost;
        } else {
            UnaryOpChecker chk(log, values, val, loc);
            node.prefixOps[npre - ipre - 1]->AcceptVisitor(chk);
            if (!chk.HasResult()) return;
            pure = pure && chk.Pure();
            val = chk.Result();
            auto iloc = node.prefixOps[npre - ipre - 1]->pos;
            loc = locators::SpanLocator(loc, iloc);
            precomp = precomp && pure && val.index() && !get<1>(val)->TypeOfValue()->Mutable();
            if (precomp) {
                node.prefixOps.pop_back();
                npre--;
                node.expr = make_shared<ast::PrecomputedValue>(loc, get<1>(val));
            } else
                ++ipre;
        }
    }
    this->res = val;
    if (node.prefixOps.empty() && node.postfixOps.empty()) this->replacement = node.expr;
}

void ExpressionChecker::VisitUnaryNot(ast::UnaryNot& node) {
    ExpressionChecker rec(log, values);
    node.nested->AcceptVisitor(rec);
    if (!rec.HasResult()) return;
    if (rec.replacement) node.nested = rec.AssertReplacementAsExpression();
    pure = rec.Pure() && !isUnknown(rec.Result());
    auto result = rec.Result();
    if (result.index()) {
        auto& rval = get<1>(result);
        runtime::RuntimeValueResult negated = get<1>(result);
        if (!negated) {
            errors::VectorOfSpanTypes bad{{node.nested->pos, rval->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>("not", bad));
            return;
        }
        if (negated->index()) {
            log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*negated).what()));
            return;
        }
        this->res = get<0>(*negated);
        if (pure) this->replacement = make_shared<ast::PrecomputedValue>(node.pos, get<0>(*negated));
        return;
    }
    auto type = get<0>(result);
    auto restype = type->UnaryNot();
    if (!restype) {
        errors::VectorOfSpanTypes bad{{node.nested->pos, type}};
        log.Log(make_shared<errors::OperatorNotApplicable>("not", bad));
        return;
    }
    this->res = *restype;
}

void ExpressionChecker::VisitPrimaryIdent(ast::PrimaryIdent& node) {
    auto val = values.LookupVariable(node.name->identifier);
    if (!val) {
        log.Log(make_shared<errors::VariableNotDefined>(node.pos, node.name->identifier));
        return;
    }
    if (val->index()) replacement = make_shared<ast::PrecomputedValue>(node.pos, get<1>(*val));
    auto valtype = val->index() ? get<1>(*val)->TypeOfValue() : get<0>(*val);
    if (valtype->TypeEq(runtime::NoneType()))
        log.Log(make_shared<errors::NoneValueAccessed>(node.pos, node.name->identifier));
    this->res = *val;
}

void ExpressionChecker::VisitParenthesesExpression(ast::ParenthesesExpression& node) {
    replacement = node.expr;
    node.expr->AcceptVisitor(*this);
}

// Tuples are mutable, cannot precompute
void ExpressionChecker::VisitTupleLiteral(ast::TupleLiteral& node) {
    bool badnames = false;
    size_t n = node.elements.size();
    {
        map<string, vector<locators::SpanLocator>> locs;
        for (auto& elem : node.elements)
            if (elem->ident) {
                auto span = elem->ident.value()->span;
                locs[elem->ident.value()->identifier].push_back(
                    locators::SpanLocator(node.pos.File(), span.position, span.length));
            }
        for (auto& kv : locs) {
            if (kv.second.size() > 2) {
                log.Log(make_shared<errors::DuplicateFieldNames>(kv.first, kv.second));
                badnames = true;
            }
        }
    }
    bool errored = false;
    for (size_t i = 0; i < n; i++) {
        auto& expr = node.elements[i]->expression;
        ExpressionChecker rec(log, values);
        expr->AcceptVisitor(rec);
        if (!rec.HasResult()) {
            errored = true;
            continue;
        }
        pure = pure && rec.Pure();
        if (rec.Replacement()) expr = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
    }
    if (errored || badnames) return;
    this->res = make_shared<runtime::TupleType>();
}

void ExpressionChecker::VisitFuncLiteral(ast::FuncLiteral& node) {
    vector<string> paramnames;
    {
        bool badnames = false;
        map<string, vector<locators::SpanLocator>> locs;
        for (auto& param : node.parameters) {
            auto span = param->span;
            locs[param->identifier].emplace_back(node.pos.File(), span.position, span.length);
            paramnames.push_back(param->identifier);
        }
        for (auto& kv : locs) {
            if (kv.second.size() <= 1) continue;
            badnames = true;
            log.Log(make_shared<errors::DuplicateParameterNames>(kv.first, kv.second));
        }
        if (badnames) return;
    }

    ValueTimeline tl(values);
    tl.StartBlindScope();
    for (auto& param : node.parameters) {
        auto span = param->span;
        auto loc = locators::SpanLocator(node.pos.File(), span.position, span.length);
        tl.Declare(param->identifier, loc);
        tl.AssignType(param->identifier, make_shared<runtime::UnknownType>(), loc);
    }

    {
        auto shrt = dynamic_pointer_cast<ast::ShortFuncBody>(node.funcBody);
        if (shrt) {
            auto bodypos = shrt->expressionToReturn->pos;
            node.funcBody = make_shared<ast::LongFuncBody>(
                node.funcBody->pos,
                make_shared<ast::Body>(bodypos, vector<shared_ptr<ast::Statement>>{make_shared<ast::ReturnStatement>(
                                                    bodypos, shrt->expressionToReturn)}));
        }
    }

    StatementChecker chk(log, tl, true, false);
    dynamic_pointer_cast<ast::LongFuncBody>(node.funcBody)->funcBody->AcceptVisitor(chk);
    if (chk.Terminated() == StatementChecker::TerminationKind::Errored) return;
    auto paraminfo = tl.EndScope();
    for (auto& unused : paraminfo.uselessAssignments)
        log.Log(make_shared<errors::AssignedValueUnused>(unused.second, unused.first));
    vector<string> captured(paraminfo.referencedExternals.size());
    std::ranges::transform(paraminfo.referencedExternals, captured.begin(),
                           [](const pair<const string, bool>& kv) { return kv.first; });
    for (auto& name : captured)
        values.AssignUnknownButUsed(name);
    auto optreturnedtype = chk.Returned();
    auto returnedtype = optreturnedtype ? *optreturnedtype : make_shared<runtime::NoneType>();
    auto functype = make_shared<runtime::FuncType>(
        chk.Pure(), vector<shared_ptr<runtime::Type>>(node.parameters.size(), make_shared<runtime::UnknownType>()),
        returnedtype);
    replacement = make_shared<ast::ClosureDefinition>(node.pos, functype, node.funcBody, paramnames, captured);
    this->res = functype;
}

void ExpressionChecker::VisitTokenLiteral(ast::TokenLiteral& node) {
    shared_ptr<runtime::RuntimeValue> val;
    switch (node.kind) {
        case ast::TokenLiteral::TokenLiteralKind::String:
            val = make_shared<runtime::StringValue>(dynamic_cast<StringLiteral&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::Int:
            val = make_shared<runtime::IntegerValue>(dynamic_cast<IntegerToken&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::Real:
            val = make_shared<runtime::RealValue>(dynamic_cast<RealToken&>(*node.token).value);
            break;
        case ast::TokenLiteral::TokenLiteralKind::True:
            val = make_shared<runtime::BoolValue>(true);
            break;
        case ast::TokenLiteral::TokenLiteralKind::False:
            val = make_shared<runtime::BoolValue>(false);
            break;
        default:  // case ast::TokenLiteral::TokenLiteralKind::None:
            val = make_shared<runtime::NoneValue>();
            break;
    }
    replacement = make_shared<ast::PrecomputedValue>(node.pos, val);
    res = val;
}

// Arrays are mutable, cannot precompute
void ExpressionChecker::VisitArrayLiteral(ast::ArrayLiteral& node) {
    size_t n = node.items.size();
    bool errored = false;
    for (size_t i = 0; i < n; i++) {
        auto& expr = node.items[i];
        ExpressionChecker rec(log, values);
        expr->AcceptVisitor(rec);
        if (!rec.HasResult()) {
            errored = true;
            continue;
        }
        pure = pure && rec.Pure();
        if (rec.Replacement()) expr = rec.AssertReplacementAsExpression();
    }
    if (errored) return;
    this->res = make_shared<runtime::ArrayType>();
}

void ExpressionChecker::VisitCustom([[maybe_unused]] ast::ASTNode& node) {
    throw std::runtime_error("Somehow visiting a Custom node in ExpressionChecker");
}

}  // namespace semantic
