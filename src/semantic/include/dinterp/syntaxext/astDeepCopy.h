#pragma once
#include "dinterp/syntax.h"

namespace dinterp {
namespace ast {

class AstDeepCopier : public IASTVisitor {
public:
    static std::shared_ptr<ASTNode> Clone(ASTNode& node);
    std::shared_ptr<ASTNode> Result;
    void VisitBody(Body& node) override;
    void VisitVarStatement(VarStatement& node) override;
    void VisitIfStatement(IfStatement& node) override;
    void VisitShortIfStatement(ShortIfStatement& node) override;
    void VisitWhileStatement(WhileStatement& node) override;
    void VisitForStatement(ForStatement& node) override;
    void VisitLoopStatement(LoopStatement& node) override;
    void VisitExitStatement(ExitStatement& node) override;
    void VisitAssignStatement(AssignStatement& node) override;
    void VisitPrintStatement(PrintStatement& node) override;
    void VisitReturnStatement(ReturnStatement& node) override;
    void VisitExpressionStatement(ExpressionStatement& node) override;
    void VisitCommaExpressions(CommaExpressions& node) override;
    void VisitCommaIdents(CommaIdents& node) override;
    void VisitIdentMemberAccessor(IdentMemberAccessor& node) override;
    void VisitIntLiteralMemberAccessor(IntLiteralMemberAccessor& node) override;
    void VisitParenMemberAccessor(ParenMemberAccessor& node) override;
    void VisitIndexAccessor(IndexAccessor& node) override;
    void VisitReference(Reference& node) override;
    void VisitXorOperator(XorOperator& node) override;
    void VisitOrOperator(OrOperator& node) override;
    void VisitAndOperator(AndOperator& node) override;
    void VisitBinaryRelation(BinaryRelation& node) override;
    void VisitSum(Sum& node) override;
    void VisitTerm(Term& node) override;
    void VisitUnary(Unary& node) override;
    void VisitUnaryNot(UnaryNot& node) override;
    void VisitPrefixOperator(PrefixOperator& node) override;
    void VisitTypecheckOperator(TypecheckOperator& node) override;
    void VisitCall(Call& node) override;
    void VisitAccessorOperator(AccessorOperator& node) override;
    void VisitPrimaryIdent(PrimaryIdent& node) override;
    void VisitParenthesesExpression(ParenthesesExpression& node) override;
    void VisitTupleLiteralElement(TupleLiteralElement& node) override;
    void VisitTupleLiteral(TupleLiteral& node) override;
    void VisitShortFuncBody(ShortFuncBody& node) override;
    void VisitLongFuncBody(LongFuncBody& node) override;
    void VisitFuncLiteral(FuncLiteral& node) override;
    void VisitTokenLiteral(TokenLiteral& node) override;
    void VisitArrayLiteral(ArrayLiteral& node) override;
    void VisitCustom(ASTNode& node) override;
    virtual ~AstDeepCopier() override = default;
};

}  // namespace ast
}
