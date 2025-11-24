#include "dinterp/semantic/unaryOpsChecker.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include "dinterp/runtime/types.h"
#include "dinterp/runtime/values.h"
#include "dinterp/semantic/diagnostics.h"
#include "dinterp/semantic/expressionChecker.h"
#include "dinterp/syntax.h"
using namespace std;

namespace semantic {

static inline bool isUnknown(const runtime::TypeOrValue& res) {
    return !res.index() && get<0>(res)->TypeEq(runtime::UnknownType());
}

UnaryOpChecker::UnaryOpChecker(complog::ICompilationLog& log, ValueTimeline& values,
                               const runtime::TypeOrValue& curvalue, const locators::SpanLocator& pos)
    : log(log), values(values), curvalue(curvalue), pos(pos), pure(!isUnknown(curvalue)) {}
bool UnaryOpChecker::HasResult() const { return static_cast<bool>(res); }
bool UnaryOpChecker::Pure() const { return pure; }
variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>> UnaryOpChecker::Result() const { return *res; }

#define DISALLOWED_VISIT(name)                                           \
    void UnaryOpChecker::Visit##name([[maybe_unused]] ast::name& node) { \
        throw runtime_error("UnaryOpChecker cannot visit ast::" #name);  \
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
DISALLOWED_VISIT(Reference)
DISALLOWED_VISIT(XorOperator)
DISALLOWED_VISIT(OrOperator)
DISALLOWED_VISIT(AndOperator)
DISALLOWED_VISIT(BinaryRelation)
DISALLOWED_VISIT(Sum)
DISALLOWED_VISIT(Term)
DISALLOWED_VISIT(Unary)
DISALLOWED_VISIT(UnaryNot)
DISALLOWED_VISIT(PrimaryIdent)
DISALLOWED_VISIT(ParenthesesExpression)
DISALLOWED_VISIT(TupleLiteralElement)
DISALLOWED_VISIT(TupleLiteral)
DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)
DISALLOWED_VISIT(FuncLiteral)
DISALLOWED_VISIT(TokenLiteral)
DISALLOWED_VISIT(ArrayLiteral)

void UnaryOpChecker::VisitCustom([[maybe_unused]] ast::ASTNode& node) {
    throw runtime_error("UnaryOpChecker cannot visit ast::Custom");
}

void UnaryOpChecker::VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) {
    if (curvalue.index()) {
        const shared_ptr<runtime::RuntimeValue>& rval = get<1>(curvalue);
        runtime::RuntimeValueResult optmember = rval->Field(node.name->identifier);
        if (!optmember) {
            log.Log(make_shared<errors::NoSuchField>(node.pos, rval->TypeOfValue(), node.name->identifier));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    auto optmember = rtype->Field(node.name->identifier);
    if (!optmember) {
        log.Log(make_shared<errors::NoSuchField>(node.pos, rtype, node.name->identifier));
        return;
    }
    res = *optmember;
}

void UnaryOpChecker::VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) {
    if (curvalue.index()) {
        runtime::IntegerValue index(node.index->value);
        const shared_ptr<runtime::RuntimeValue>& rval = get<1>(curvalue);
        runtime::RuntimeValueResult optmember = rval->Field(index);
        if (!optmember) {
            log.Log(make_shared<errors::NoSuchField>(node.pos, rval->TypeOfValue(), node.index->value.ToString()));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    auto optmember = rtype->Field(runtime::IntegerType());
    if (!optmember) {
        log.Log(make_shared<errors::NoSuchField>(node.pos, rtype, node.index->value.ToString()));
        return;
    }
    res = *optmember;
}

void UnaryOpChecker::VisitParenMemberAccessor(ast::ParenMemberAccessor& node) {
    ExpressionChecker nested(log, values);
    node.expr->AcceptVisitor(nested);
    if (!nested.HasResult()) return;
    pure = nested.Pure();
    if (nested.Replacement()) node.expr = nested.AssertReplacementAsExpression();
    auto nestedres = nested.Result();
    if (nestedres.index() && curvalue.index()) {
        shared_ptr<runtime::RuntimeValue> index(get<1>(nestedres));
        const shared_ptr<runtime::RuntimeValue>& rval = get<1>(curvalue);
        runtime::RuntimeValueResult optmember = rval->Field(*index);
        if (!optmember) {
            if (index->TypeOfValue()->TypeEq(runtime::IntegerType())) {
                log.Log(make_shared<errors::NoSuchField>(
                    node.pos, rval->TypeOfValue(), dynamic_cast<runtime::IntegerValue&>(*index).Value().ToString()));
                return;
            }
            errors::VectorOfSpanTypes bad{{pos, rval->TypeOfValue()}, {node.pos, index->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>(".", bad));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    shared_ptr<runtime::Type> nestedtype;
    if (nestedres.index())
        nestedtype = get<1>(nestedres)->TypeOfValue();
    else
        nestedtype = get<0>(nestedres);
    auto optmember = rtype->Field(*nestedtype);
    if (!optmember) {
        errors::VectorOfSpanTypes bad{{pos, rtype}, {node.pos, nestedtype}};
        log.Log(make_shared<errors::OperatorNotApplicable>(".", bad));
        return;
    }
    res = *optmember;
}

void UnaryOpChecker::VisitIndexAccessor(ast::IndexAccessor& node) {
    ExpressionChecker nested(log, values);
    node.expressionInBrackets->AcceptVisitor(nested);
    if (!nested.HasResult()) return;
    pure = nested.Pure();
    if (nested.Replacement()) node.expressionInBrackets = nested.AssertReplacementAsExpression();
    auto nestedres = nested.Result();
    if (nestedres.index() && curvalue.index()) {
        shared_ptr<runtime::RuntimeValue> index(get<1>(nestedres));
        const shared_ptr<runtime::RuntimeValue>& rval = get<1>(curvalue);
        runtime::RuntimeValueResult optmember = rval->Subscript(*index);
        if (!optmember) {
            errors::VectorOfSpanTypes bad{{pos, rval->TypeOfValue()}, {node.pos, index->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>("[subscript]", bad));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    shared_ptr<runtime::Type> nestedtype;
    if (nestedres.index())
        nestedtype = get<1>(nestedres)->TypeOfValue();
    else
        nestedtype = get<0>(nestedres);
    auto optmember = rtype->Subscript(*nestedtype);
    if (!optmember) {
        errors::VectorOfSpanTypes bad{{pos, rtype}, {node.pos, nestedtype}};
        log.Log(make_shared<errors::OperatorNotApplicable>("[subscript]", bad));
        return;
    }
    res = *optmember;
}

// A call makes every variable 'unknown'
void UnaryOpChecker::VisitCall(ast::Call& node) {
    size_t n = node.args.size();
    vector<optional<shared_ptr<runtime::RuntimeValue>>> optValues;
    optValues.reserve(n);
    vector<shared_ptr<runtime::Type>> types;
    types.reserve(n);
    bool errored = false;
    bool allknown = true;
    for (size_t i = 0; i < n; i++) {
        ExpressionChecker rec(log, values);
        shared_ptr<ast::Expression>& arg = node.args[i];
        arg->AcceptVisitor(rec);
        if (!rec.HasResult()) {
            errored = true;
            continue;
        }
        pure = pure && rec.Pure() && !isUnknown(rec.Result());
        if (rec.Replacement()) arg = rec.AssertReplacementAsExpression();
        auto res = rec.Result();
        if (res.index()) {
            const auto& val = get<1>(res);
            optValues.emplace_back(val);
            types.push_back(val->TypeOfValue());
        } else {
            optValues.emplace_back();
            types.push_back(get<0>(res));
            allknown = false;
        }
    }
    if (errored) return;
    if (curvalue.index() && allknown && pure) {
        auto& rval = *get<1>(curvalue);
        auto curtype = rval.TypeOfValue();
        if (!curtype->TypeEq(runtime::FuncType())) {
            log.Log(make_shared<errors::TriedToCallNonFunction>(pos, curtype));
            return;
        }
        auto& funcvalue = dynamic_cast<runtime::FuncValue&>(rval);
        auto& functype = dynamic_cast<runtime::FuncType&>(*curtype);
        pure = pure && functype.Pure();
        if (pure) {
            vector<shared_ptr<runtime::RuntimeValue>> args(n);
            std::ranges::transform(optValues, args.begin(),
                                   [](const optional<shared_ptr<runtime::RuntimeValue>> a) { return *a; });
            auto res = funcvalue.Call(args);
            if (!res) {
                errors::VectorOfSpanTypes bad{{pos, curtype}};
                log.Log(make_shared<errors::OperatorNotApplicable>("(call)", bad));
                return;
            }
            if (res->index()) {
                log.Log(make_shared<errors::EvaluationException>(node.pos, get<1>(*res).what()));
                return;
            }
            this->res = get<0>(*res);
            if (!functype.Pure()) values.MakeAllUnknown();
            return;
        }
    }
    auto mytype = curvalue.index() ? get<1>(curvalue)->TypeOfValue() : get<0>(curvalue);
    if (mytype->TypeEq(runtime::UnknownType())) {
        this->res = make_shared<runtime::UnknownType>();
        pure = false;
        values.MakeAllUnknown();
        return;
    }
    shared_ptr<runtime::FuncType> functype = dynamic_pointer_cast<runtime::FuncType>(mytype);
    if (!functype) {
        log.Log(make_shared<errors::TriedToCallNonFunction>(pos, mytype));
        return;
    }
    pure = pure && functype->Pure();
    auto needtypes = functype->ArgTypes();
    if (needtypes) {
        const size_t needcount = needtypes->size();
        if (needcount != n) {
            log.Log(make_shared<errors::WrongArgumentCount>(node.pos, needcount, n));
            return;
        }
        for (size_t i = 0; i < n; i++) {
            auto& neededtype = needtypes->at(i);
            if (neededtype->TypeEq(runtime::UnknownType())) continue;
            auto& giventype = types[i];
            if (giventype->TypeEq(runtime::UnknownType()) || neededtype->TypeEq(*giventype)) continue;
            log.Log(make_shared<errors::WrongArgumentType>(node.args[i]->pos, neededtype, giventype));
            errored = true;
        }
        if (errored) return;
    }
    this->res = functype->ReturnType();
    if (!functype->Pure()) values.MakeAllUnknown();
}

void UnaryOpChecker::VisitAccessorOperator(ast::AccessorOperator& node) { node.accessor->AcceptVisitor(*this); }

void UnaryOpChecker::VisitPrefixOperator(ast::PrefixOperator& node) {
    const char* const OPERATOR_NAMES[] = {"unary+", "unary-"};
    const char* const OPERATOR_NAME = OPERATOR_NAMES[static_cast<int>(node.kind)];
    if (curvalue.index()) {
        auto& rval = get<1>(curvalue);
        runtime::RuntimeValueResult res =
            (node.kind == ast::PrefixOperator::PrefixOperatorKind::Plus) ? rval->UnaryPlus() : rval->UnaryMinus();
        if (!res) {
            errors::VectorOfSpanTypes bad{{pos, rval->TypeOfValue()}};
            log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, bad));
            return;
        }
        if (res->index()) {
            log.Log(make_shared<errors::EvaluationException>(pos, get<1>(*res).what()));
            return;
        }
        this->res = get<0>(*res);
        return;
    }
    auto& type = get<0>(curvalue);
    auto res = (node.kind == ast::PrefixOperator::PrefixOperatorKind::Plus) ? type->UnaryPlus() : type->UnaryMinus();
    if (!res) {
        errors::VectorOfSpanTypes bad{{pos, type}};
        log.Log(make_shared<errors::OperatorNotApplicable>(OPERATOR_NAME, bad));
        return;
    }
    this->res = *res;
}

void UnaryOpChecker::VisitTypecheckOperator(ast::TypecheckOperator& node) {
    auto type = curvalue.index() ? get<1>(curvalue)->TypeOfValue() : get<0>(curvalue);
    if (type->TypeEq(runtime::UnknownType())) {
        this->res = make_shared<runtime::BoolType>();
        return;
    }
    unique_ptr<runtime::Type> types[] = {make_unique<runtime::IntegerType>(), make_unique<runtime::RealType>(),
                                         make_unique<runtime::StringType>(),  make_unique<runtime::BoolType>(),
                                         make_unique<runtime::NoneType>(),    make_unique<runtime::FuncType>(),
                                         make_unique<runtime::TupleType>(),   make_unique<runtime::ArrayType>()};
    this->res = make_shared<runtime::BoolValue>(type->TypeEq(*types[static_cast<int>(node.typeId)]));
}

UnaryOpChecker::~UnaryOpChecker() {}

}  // namespace semantic
