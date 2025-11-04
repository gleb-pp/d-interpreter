#include "unaryOpsChecker.h"
#include <algorithm>
#include <stdexcept>
#include "expressionChecker.h"
#include "runtime/types.h"
#include "runtime/values.h"
#include "diagnostics.h"
#include "syntax.h"
using namespace std;

UnaryOpChecker::UnaryOpChecker(complog::ICompilationLog& log, ValueTimeline& values,
               const variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>>& curvalue,
               const locators::SpanLocator& pos)
    : log(log), values(values), curvalue(curvalue), pos(pos) {}
bool UnaryOpChecker::HasResult() const { return static_cast<bool>(res); }
bool UnaryOpChecker::Pure() const { return pure; }
variant<shared_ptr<runtime::Type>, shared_ptr<runtime::RuntimeValue>> UnaryOpChecker::Result() const { return *res; }

#define DISALLOWED_VISIT(name)                                          \
    void UnaryOpChecker::Visit##name(ast::name& node) {                 \
        throw runtime_error("UnaryOpChecker cannot visit ast::" #name); \
    }
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
DISALLOWED_VISIT(PrefixOperator)
DISALLOWED_VISIT(TypecheckOperator)
DISALLOWED_VISIT(PrimaryIdent)
DISALLOWED_VISIT(ParenthesesExpression)
DISALLOWED_VISIT(TupleLiteralElement)
DISALLOWED_VISIT(TupleLiteral)
DISALLOWED_VISIT(ShortFuncBody)
DISALLOWED_VISIT(LongFuncBody)
DISALLOWED_VISIT(FuncLiteral)
DISALLOWED_VISIT(TokenLiteral)
DISALLOWED_VISIT(ArrayLiteral)
void UnaryOpChecker::VisitCustom(ast::ASTNode& node) {
    throw runtime_error(
        "UnaryOpChecker cannot visit ast::Custom");
}

void UnaryOpChecker::VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) {
    if (curvalue.index()) {
        const shared_ptr<runtime::RuntimeValue>& rval = get<1>(curvalue);
        runtime::RuntimeValueResult optmember = rval->Field(node.name->identifier);
        if (!optmember) {
            log.Log(make_shared<semantic_errors::NoSuchField>(node.pos, rval->TypeOfValue(), node.name->identifier));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<semantic_errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    auto optmember = rtype->Field(node.name->identifier);
    if (!optmember) {
        log.Log(make_shared<semantic_errors::NoSuchField>(node.pos, rtype, node.name->identifier));
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
            log.Log(make_shared<semantic_errors::NoSuchField>(node.pos, rval->TypeOfValue(),
                                                              node.index->value.ToString()));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<semantic_errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    auto optmember = rtype->Field(runtime::IntegerType());
    if (!optmember) {
        log.Log(make_shared<semantic_errors::NoSuchField>(node.pos, rtype, node.index->value.ToString()));
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
            if (index->TypeOfValue()->TypeEq(runtime::IntegerType()))
            {
                log.Log(make_shared<semantic_errors::NoSuchField>(
                    node.pos, rval->TypeOfValue(), dynamic_cast<runtime::IntegerValue&>(*index).Value().ToString()));
                return;
            }
            semantic_errors::VectorOfSpanTypes bad{{pos, rval->TypeOfValue()}, {node.pos, index->TypeOfValue()}};
            log.Log(make_shared<semantic_errors::OperatorNotApplicable>(".", bad));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<semantic_errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    shared_ptr<runtime::Type> nestedtype;
    if (nestedres.index()) nestedtype = get<1>(nestedres)->TypeOfValue();
    else nestedtype = get<0>(nestedres);
    auto optmember = rtype->Field(*nestedtype);
    if (!optmember) {
        semantic_errors::VectorOfSpanTypes bad{{pos, rtype}, {node.pos, nestedtype}};
        log.Log(make_shared<semantic_errors::OperatorNotApplicable>(".", bad));
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
            semantic_errors::VectorOfSpanTypes bad{{pos, rval->TypeOfValue()}, {node.pos, index->TypeOfValue()}};
            log.Log(make_shared<semantic_errors::OperatorNotApplicable>("[subscript]", bad));
            return;
        }
        if (optmember->index()) {
            log.Log(make_shared<semantic_errors::EvaluationException>(node.pos, get<1>(*optmember).what()));
            return;
        }
        res = get<0>(*optmember);
        return;
    }
    const shared_ptr<runtime::Type>& rtype = get<0>(curvalue);
    shared_ptr<runtime::Type> nestedtype;
    if (nestedres.index()) nestedtype = get<1>(nestedres)->TypeOfValue();
    else nestedtype = get<0>(nestedres);
    auto optmember = rtype->Subscript(*nestedtype);
    if (!optmember) {
        semantic_errors::VectorOfSpanTypes bad{{pos, rtype}, {node.pos, nestedtype}};
        log.Log(make_shared<semantic_errors::OperatorNotApplicable>("[subscript]", bad));
        return;
    }
    res = *optmember;
}

void UnaryOpChecker::VisitCall(ast::Call& node) {
    size_t n = node.args.size();
    vector<optional<shared_ptr<runtime::RuntimeValue>>> optValues(n);
    vector<shared_ptr<runtime::Type>> types(n);
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
        pure = pure && rec.Pure();
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
            log.Log(make_shared<semantic_errors::TriedToCallNonFunction>(pos, curtype));
            return;
        }
        auto& funcvalue = dynamic_cast<runtime::FuncValue&>(rval);
        auto& functype = dynamic_cast<runtime::FuncType&>(*curtype);
        pure = pure && functype.Pure();
        if (pure) {
            vector<shared_ptr<runtime::RuntimeValue>> args(n);
            std::ranges::transform(optValues, args.begin(), [](const optional<shared_ptr<runtime::RuntimeValue>> a) { return *a; });
            auto res = funcvalue.Call(args);
            if (!res) {
                semantic_errors::VectorOfSpanTypes bad{{pos, curtype}};
                log.Log(make_shared<semantic_errors::OperatorNotApplicable>("(call)", bad));
                return;
            }
            if (res->index()) {
                log.Log(make_shared<semantic_errors::EvaluationException>(node.pos, get<1>(*res).what()));
                return;
            }
            this->res = get<0>(*res);
            return;
        }
    }
    auto mytype = curvalue.index() ? get<1>(curvalue)->TypeOfValue() : get<0>(curvalue);
    if (mytype->TypeEq(runtime::UnknownType())) {
        this->res = make_shared<runtime::UnknownType>();
        pure = false;
        return;
    }
    shared_ptr<runtime::FuncType> functype = dynamic_pointer_cast<runtime::FuncType>(mytype);
    if (!functype) {
        log.Log(make_shared<semantic_errors::TriedToCallNonFunction>(pos, mytype));
        return;
    }
    pure = pure && functype->Pure();
    auto needtypes = functype->ArgTypes();
    if (needtypes) {
        if (const size_t needcount = needtypes->size(); needcount != n) {
            log.Log(make_shared<semantic_errors::WrongArgumentCount>(node.pos, needcount, n));
            return;
        }
        for (int i = 0; i < n; i++) {
            auto& neededtype = needtypes->at(i);
            auto& giventype = types[i];
            if (neededtype->TypeEq(*giventype)) continue;
            log.Log(make_shared<semantic_errors::WrongArgumentType>(node.args[i]->pos, neededtype, giventype));
            errored = true;
        }
        if (errored) return;
    }
    this->res = functype->ReturnType();
}

void UnaryOpChecker::VisitAccessorOperator(ast::AccessorOperator& node) {
    node.accessor->AcceptVisitor(*this);
}

