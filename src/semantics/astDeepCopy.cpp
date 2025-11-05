#include "syntaxext/astDeepCopy.h"

#include <algorithm>
#include <memory>

#include "syntax.h"
using namespace std;

namespace ast {

shared_ptr<ast::ASTNode> AstDeepCopier::Clone(ast::ASTNode& node) {
    AstDeepCopier cp;
    node.AcceptVisitor(cp);
    return cp.Result;
}

void AstDeepCopier::VisitBody(ast::Body& node) {
    auto res = make_shared<ast::Body>(node.pos);
    Result = res;
    auto& inner = res->statements;
    inner.resize(node.statements.size());
    std::ranges::transform(node.statements, inner.begin(), [](const shared_ptr<ast::Statement>& a) {
        AstDeepCopier cp;
        a->AcceptVisitor(cp);
        return dynamic_pointer_cast<ast::Statement>(cp.Result);
    });
}

void AstDeepCopier::VisitVarStatement(ast::VarStatement& node) {
    auto res = make_shared<ast::VarStatement>(node);
    for (auto& def : res->definitions) {
        if (def.second) {
            auto& expr = *def.second;
            expr->AcceptVisitor(*this);
            expr = dynamic_pointer_cast<ast::Expression>(Result);
        }
    }
    Result = res;
}

void AstDeepCopier::VisitIfStatement(ast::IfStatement& node) {
    auto res = make_shared<ast::IfStatement>(node);
    res->condition->AcceptVisitor(*this);
    res->condition = dynamic_pointer_cast<ast::Expression>(Result);
    res->doIfTrue->AcceptVisitor(*this);
    res->doIfTrue = dynamic_pointer_cast<ast::Body>(Result);
    if (res->doIfFalse) {
        auto& body = *res->doIfFalse;
        body->AcceptVisitor(*this);
        body = dynamic_pointer_cast<ast::Body>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitShortIfStatement(ast::ShortIfStatement& node) {
    auto res = make_shared<ast::ShortIfStatement>(node);
    res->condition->AcceptVisitor(*this);
    res->condition = dynamic_pointer_cast<ast::Expression>(Result);
    res->doIfTrue->AcceptVisitor(*this);
    res->doIfTrue = dynamic_pointer_cast<ast::Statement>(Result);
    Result = res;
}

void AstDeepCopier::VisitWhileStatement(ast::WhileStatement& node) {
    auto res = make_shared<ast::WhileStatement>(node);
    res->condition->AcceptVisitor(*this);
    res->condition = dynamic_pointer_cast<ast::Expression>(Result);
    res->action->AcceptVisitor(*this);
    res->action = dynamic_pointer_cast<ast::Body>(Result);
    Result = res;
}

void AstDeepCopier::VisitForStatement(ast::ForStatement& node) {
    auto res = make_shared<ast::ForStatement>(node);
    res->startOrList->AcceptVisitor(*this);
    res->startOrList = dynamic_pointer_cast<ast::Expression>(Result);
    if (res->end) {
        auto& expr = *res->end;
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    res->action->AcceptVisitor(*this);
    res->action = dynamic_pointer_cast<ast::Body>(Result);
    Result = res;
}

void AstDeepCopier::VisitLoopStatement(ast::LoopStatement& node) {
    auto res = make_shared<ast::LoopStatement>(node);
    res->body->AcceptVisitor(*this);
    res->body = dynamic_pointer_cast<ast::Body>(Result);
    Result = res;
}

void AstDeepCopier::VisitExitStatement(ast::ExitStatement& node) {
    auto res = make_shared<ast::ExitStatement>(node);
    Result = res;
}

void AstDeepCopier::VisitAssignStatement(ast::AssignStatement& node) {
    auto res = make_shared<ast::AssignStatement>(node);
    res->dest->AcceptVisitor(*this);
    res->dest = dynamic_pointer_cast<ast::Reference>(Result);
    res->src->AcceptVisitor(*this);
    res->src = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitPrintStatement(ast::PrintStatement& node) {
    auto res = make_shared<ast::PrintStatement>(node);
    for (auto& expr : res->expressions) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitReturnStatement(ast::ReturnStatement& node) {
    auto res = make_shared<ast::ReturnStatement>(node);
    if (res->returnValue) {
        auto& expr = *res->returnValue;
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitExpressionStatement(ast::ExpressionStatement& node) {
    auto res = make_shared<ast::ExpressionStatement>(node);
    res->expr->AcceptVisitor(*this);
    res->expr = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitCommaExpressions(ast::CommaExpressions& node) {
    auto res = make_shared<ast::CommaExpressions>(node);
    for (auto& expr : res->expressions) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitCommaIdents(ast::CommaIdents& node) {
    auto res = make_shared<ast::CommaIdents>(node);
    Result = res;
}

void AstDeepCopier::VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) {
    auto res = make_shared<ast::IdentMemberAccessor>(node);
    Result = res;
}

void AstDeepCopier::VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) {
    auto res = make_shared<ast::IntLiteralMemberAccessor>(node);
    Result = res;
}

void AstDeepCopier::VisitParenMemberAccessor(ast::ParenMemberAccessor& node) {
    auto res = make_shared<ast::ParenMemberAccessor>(node);
    res->expr->AcceptVisitor(*this);
    res->expr = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitIndexAccessor(ast::IndexAccessor& node) {
    auto res = make_shared<ast::IndexAccessor>(node);
    res->expressionInBrackets->AcceptVisitor(*this);
    res->expressionInBrackets = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitReference(ast::Reference& node) {
    auto res = make_shared<ast::Reference>(node);
    for (auto& acc : res->accessorChain) {
        acc->AcceptVisitor(*this);
        acc = dynamic_pointer_cast<ast::Accessor>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitXorOperator(ast::XorOperator& node) {
    auto res = make_shared<ast::XorOperator>(node);
    for (auto& expr : res->operands) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitOrOperator(ast::OrOperator& node) {
    auto res = make_shared<ast::OrOperator>(node);
    for (auto& expr : res->operands) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitAndOperator(ast::AndOperator& node) {
    auto res = make_shared<ast::AndOperator>(node);
    for (auto& expr : res->operands) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitBinaryRelation(ast::BinaryRelation& node) {
    auto res = make_shared<ast::BinaryRelation>(node);
    for (auto& expr : res->operands) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitSum(ast::Sum& node) {
    auto res = make_shared<ast::Sum>(node);
    for (auto& expr : res->terms) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitTerm(ast::Term& node) {
    auto res = make_shared<ast::Term>(node);
    for (auto& expr : res->unaries) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitUnary(ast::Unary& node) {
    auto res = make_shared<ast::Unary>(node);
    res->expr->AcceptVisitor(*this);
    res->expr = dynamic_pointer_cast<ast::Expression>(Result);
    for (auto& pre : res->prefixOps) {
        pre->AcceptVisitor(*this);
        pre = dynamic_pointer_cast<ast::PrefixOperator>(Result);
    }
    for (auto& post : res->postfixOps) {
        post->AcceptVisitor(*this);
        post = dynamic_pointer_cast<ast::PostfixOperator>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitUnaryNot(ast::UnaryNot& node) {
    auto res = make_shared<ast::UnaryNot>(node);
    res->nested->AcceptVisitor(*this);
    res->nested = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitPrefixOperator(ast::PrefixOperator& node) {
    auto res = make_shared<ast::PrefixOperator>(node);
    Result = res;
}

void AstDeepCopier::VisitTypecheckOperator(ast::TypecheckOperator& node) {
    auto res = make_shared<ast::TypecheckOperator>(node);
    Result = res;
}

void AstDeepCopier::VisitCall(ast::Call& node) {
    auto res = make_shared<ast::Call>(node);
    for (auto& expr : res->args) {
        expr->AcceptVisitor(*this);
        expr = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitAccessorOperator(ast::AccessorOperator& node) {
    auto res = make_shared<ast::AccessorOperator>(node);
    res->accessor->AcceptVisitor(*this);
    res->accessor = dynamic_pointer_cast<ast::Accessor>(Result);
    Result = res;
}

void AstDeepCopier::VisitPrimaryIdent(ast::PrimaryIdent& node) {
    auto res = make_shared<ast::PrimaryIdent>(node);
    Result = res;
}

void AstDeepCopier::VisitParenthesesExpression(ast::ParenthesesExpression& node) {
    auto res = make_shared<ast::ParenthesesExpression>(node);
    res->expr->AcceptVisitor(*this);
    res->expr = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitTupleLiteralElement(ast::TupleLiteralElement& node) {
    auto res = make_shared<ast::TupleLiteralElement>(node);
    res->expression->AcceptVisitor(*this);
    res->expression = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitTupleLiteral(ast::TupleLiteral& node) {
    auto res = make_shared<ast::TupleLiteral>(node);
    for (auto& elem : node.elements) {
        elem->AcceptVisitor(*this);
        elem = dynamic_pointer_cast<ast::TupleLiteralElement>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitShortFuncBody(ast::ShortFuncBody& node) {
    auto res = make_shared<ast::ShortFuncBody>(node);
    res->expressionToReturn->AcceptVisitor(*this);
    res->expressionToReturn = dynamic_pointer_cast<ast::Expression>(Result);
    Result = res;
}

void AstDeepCopier::VisitLongFuncBody(ast::LongFuncBody& node) {
    auto res = make_shared<ast::LongFuncBody>(node);
    res->funcBody->AcceptVisitor(*this);
    res->funcBody = dynamic_pointer_cast<ast::Body>(Result);
    Result = res;
}

void AstDeepCopier::VisitFuncLiteral(ast::FuncLiteral& node) {
    auto res = make_shared<ast::FuncLiteral>(node);
    res->funcBody->AcceptVisitor(*this);
    res->funcBody = dynamic_pointer_cast<ast::FuncBody>(Result);
    Result = res;
}

void AstDeepCopier::VisitTokenLiteral(ast::TokenLiteral& node) {
    auto res = make_shared<ast::TokenLiteral>(node);
    Result = res;
}

void AstDeepCopier::VisitArrayLiteral(ast::ArrayLiteral& node) {
    auto res = make_shared<ast::ArrayLiteral>(node);
    for (auto& item : node.items) {
        item->AcceptVisitor(*this);
        item = dynamic_pointer_cast<ast::Expression>(Result);
    }
    Result = res;
}

void AstDeepCopier::VisitCustom([[maybe_unused]] ast::ASTNode& node) {
    throw std::runtime_error("Cannot duplicate a Custom ast node");
}

}  // namespace ast
