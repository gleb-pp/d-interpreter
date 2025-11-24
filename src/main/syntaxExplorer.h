#pragma once
#include <iostream>
#include <vector>

#include "dinterp/syntax.h"

class ASTExplorer {
public:
    struct ActionCommand {
        std::string Command;
        std::string Description;
        ActionCommand(const std::string& command, const std::string& description);
    };
    virtual std::shared_ptr<ast::ASTNode> GetNode() const = 0;
    virtual std::vector<ActionCommand> GetActionCommands() const = 0;
    virtual std::optional<std::shared_ptr<ast::ASTNode>> Action(std::string command, std::ostream& output) const = 0;
    virtual std::string NodeName() const = 0;
    virtual ~ASTExplorer() = default;
};

class ASTExplorerVisitor : public ast::IASTVisitor {
    std::shared_ptr<const ASTExplorer> explorer;

public:
    std::shared_ptr<const ASTExplorer> MakeExplorer();
    void VisitBody(ast::Body& node) override;
    void VisitVarStatement(ast::VarStatement& node) override;
    void VisitIfStatement(ast::IfStatement& node) override;
    void VisitShortIfStatement(ast::ShortIfStatement& node) override;
    void VisitWhileStatement(ast::WhileStatement& node) override;
    void VisitForStatement(ast::ForStatement& node) override;
    void VisitLoopStatement(ast::LoopStatement& node) override;
    void VisitExitStatement(ast::ExitStatement& node) override;
    void VisitAssignStatement(ast::AssignStatement& node) override;
    void VisitPrintStatement(ast::PrintStatement& node) override;
    void VisitReturnStatement(ast::ReturnStatement& node) override;
    void VisitExpressionStatement(ast::ExpressionStatement& node) override;
    void VisitCommaExpressions(ast::CommaExpressions& node) override;
    void VisitCommaIdents(ast::CommaIdents& node) override;
    void VisitIdentMemberAccessor(ast::IdentMemberAccessor& node) override;
    void VisitIntLiteralMemberAccessor(ast::IntLiteralMemberAccessor& node) override;
    void VisitParenMemberAccessor(ast::ParenMemberAccessor& node) override;
    void VisitIndexAccessor(ast::IndexAccessor& node) override;
    void VisitReference(ast::Reference& node) override;
    void VisitXorOperator(ast::XorOperator& node) override;
    void VisitOrOperator(ast::OrOperator& node) override;
    void VisitAndOperator(ast::AndOperator& node) override;
    void VisitBinaryRelation(ast::BinaryRelation& node) override;
    void VisitSum(ast::Sum& node) override;
    void VisitTerm(ast::Term& node) override;
    void VisitUnary(ast::Unary& node) override;
    void VisitUnaryNot(ast::UnaryNot& node) override;
    void VisitPrefixOperator(ast::PrefixOperator& node) override;
    void VisitTypecheckOperator(ast::TypecheckOperator& node) override;
    void VisitCall(ast::Call& node) override;
    void VisitAccessorOperator(ast::AccessorOperator& node) override;
    void VisitPrimaryIdent(ast::PrimaryIdent& node) override;
    void VisitParenthesesExpression(ast::ParenthesesExpression& node) override;
    void VisitTupleLiteralElement(ast::TupleLiteralElement& node) override;
    void VisitTupleLiteral(ast::TupleLiteral& node) override;
    void VisitShortFuncBody(ast::ShortFuncBody& node) override;
    void VisitLongFuncBody(ast::LongFuncBody& node) override;
    void VisitFuncLiteral(ast::FuncLiteral& node) override;
    void VisitTokenLiteral(ast::TokenLiteral& node) override;
    void VisitArrayLiteral(ast::ArrayLiteral& node) override;
    void VisitCustom(ast::ASTNode& node) override;
    virtual ~ASTExplorerVisitor() override = default;
};

class ExplorerIO {
    std::shared_ptr<ast::ASTNode> rootNode;

public:
    ExplorerIO(const std::shared_ptr<ast::ASTNode>& root);
    static void PrintCommands(const ASTExplorer& explorer, std::ostream& output);
    void Explore(std::ostream& output, std::istream& input);
};
