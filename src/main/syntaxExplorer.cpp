#include "syntaxExplorer.h"
#include "syntax.h"
#include <memory>
#include <sstream>
#include <string>
using namespace std;

static int StrToInt(const std::string& s) {
    istringstream ss(s);
    int res; ss >> res;
    return res;
}

ASTExplorer::ActionCommand::ActionCommand(const string& command, const string& description)
    : Command(command), Description(description) {}

#define EXPLORER_HELP(classname, _gotoactions, _action, literalExplorer)\
class classname##literalExplorer : public ASTExplorer {\
    shared_ptr<ast::classname> node;\
public:\
    classname##literalExplorer(const shared_ptr<ast::classname>& node) : node(node) {}\
    vector<ActionCommand> GetActionCommands() const override _gotoactions\
    optional<shared_ptr<ast::ASTNode>> Action(string command, [[maybe_unused]] ostream& output) const override _action\
    string NodeName() const override { return #classname; }\
    virtual ~classname##literalExplorer() override = default;\
};

#define EXPLORER(classname, _gotoactions, _action)\
    EXPLORER_HELP(classname, _gotoactions, _action, Explorer)

EXPLORER(Body, { // GetActionCommands
    vector<ActionCommand> res;
    int i = 0;
    for (const auto& pstatement : node->statements) {
        res.emplace_back(to_string(i), "statements[" + to_string(i) + "]");
        i++;
    }
    return res;
},
{  // Action
    return node->statements[StrToInt(command)];
})

EXPLORER(VarStatement, {  // GetActionCommands
    vector<ActionCommand> res;
    int i = 0;
    for (const auto& p : node->definitions) {
        res.emplace_back("n" + to_string(i), "Name  of definitions[" + to_string(i) + "]");
        if (p.second.has_value())
            res.emplace_back("v" + to_string(i), "Value of definitions[" + to_string(i) + "]");
        i++;
    }
    return res;
},
{  // Action
    bool name = command[0] == 'n';
    command.erase(0, 1);
    auto& p = node->definitions[StrToInt(command)];
    if (name) {
        output << p.first;
        return {};
    }
    else
        return *p.second;
})

EXPLORER(IfStatement, {  // GetActionCommands
    vector<ActionCommand> res({ { "c", "Condition" }, { "t", "Do if true" } });
    if (node->doIfFalse.has_value())
        res.emplace_back("f", "Do if false");
    return res;
},
{  // Action
    switch (command[0]) {
    case 'c':
        return node->condition;
    case 't':
        return node->doIfTrue;
    case 'f':
        return node->doIfFalse;
    }
    return {};
})

EXPLORER(ShortIfStatement, {  // GetActionCommands
    return vector<ActionCommand>({ { "c", "Condition" }, { "t", "Do if true" } });
},
{  // Action
    if (command[0] == 'c') return node->condition;
    return node->doIfTrue;
})

EXPLORER(WhileStatement, {  // GetActionCommands
    return vector<ActionCommand>({ { "c", "Condition" }, { "a", "Action" } });
},
{  // Action
    if (command[0] == 'c') return node->condition;
    return node->action;
})

EXPLORER(ForStatement, {  // GetActionCommands
    vector<ActionCommand> res;
    if (node->optVariableName.has_value())
        res.emplace_back("var", "Cycle variable name");
    if (node->end.has_value()) {
        res.emplace_back("start", "Starting value");
        res.emplace_back("end", "Last value");
    } else
        res.emplace_back("list", "Iterable");
    return res;
},
{  // Action
    if (command == "var") {
        output << node->optVariableName.value()->identifier;
        return {};
    }
    if (command == "start" || command == "list") return node->startOrList;
    return *node->end;
})

EXPLORER(LoopStatement, {  // GetActionCommands
    return vector<ActionCommand>({ { "b", "Loop body" } });
},
{  // Action
    return node->body;
})

EXPLORER(ExitStatement, {  // GetActionCommands
    return {};
},
{  // Action
    return {};
})

EXPLORER(AssignStatement, {  // GetActionCommands
    return vector<ActionCommand>({
        { "d", "Destination (left-hand side)" },
        { "l", "Destination (left-hand side)" },
        { "s", "Source      (right-hand side)" },
        { "r", "Source      (right-hand side)" } });
},
{  // Action
    switch (command[0]) {
    case 'd': case 'l': return node->dest;
    default: return node->src;
    }
})

EXPLORER(PrintStatement, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->expressions.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "expressions[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->expressions[StrToInt(command)];
})

EXPLORER(ReturnStatement, {  // GetActionCommands
    return vector<ActionCommand>({ { "v", "Return value" } });
},
{  // Action
    return node->returnValue;
})

EXPLORER(ExpressionStatement, {  // GetActionCommands
    return vector<ActionCommand>({ { "e", "The expression" } });
},
{  // Action
    return node->expr;
})

EXPLORER(EmptyStatement, {  // GetActionCommands
    return {};
},
{  // Action
    return {};
})

EXPLORER(CommaExpressions, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->expressions.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "expressions[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->expressions[StrToInt(command)];
})

EXPLORER(CommaIdents, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->idents.size();
    for (int i = 0; i < n; ++i)
        res.emplace_back(to_string(i), "idents[" + to_string(i) + "]");
    return res;
},
{  // Action
    output << node->idents[StrToInt(command)]->identifier;
    return {};
})

EXPLORER(IdentMemberAccessor, {  // GetActionCommands
    return vector<ActionCommand>({ { "i", "The identifier" } });
},
{  // Action
    output << node->name->identifier;
    return {};
})

EXPLORER(IntLiteralMemberAccessor, {  // GetActionCommands
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    output << node->index->value;
    return {};
})

EXPLORER(ParenMemberAccessor, {  // GetActionCommands
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    return node->expr;
})

EXPLORER(IndexAccessor, {  // GetActionCommands
    return vector<ActionCommand>({ { "i", "The index" } });
},
{  // Action
    return node->expressionInBrackets;
})

EXPLORER(Reference, {  // GetActionCommands
    vector<ActionCommand> res({ { "b", "The base identifier" } });
    int n = node->accessorChain.size();
    for (int i = 0; i < n; ++i) res.emplace_back("a" + to_string(i), "accessor[" + to_string(i) + "]");
    return res;
},
{  // Action
    bool base = command[0] == 'b';
    if (base) {
        output << node->baseIdent;
        return {};
    } else {
        command.erase(0, 1);
        return node->accessorChain[StrToInt(command)];
    }
})

EXPLORER(Expression, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "XOR operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

EXPLORER(OrOperator, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "OR operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

EXPLORER(AndOperator, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "AND operands[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->operands[StrToInt(command)];
})

static string to_string(ast::BinaryRelationOperator op) {
    switch (op) {
        case ast::BinaryRelationOperator::Equal: return     "Equal     =";
        case ast::BinaryRelationOperator::Less: return      "Less      <";
        case ast::BinaryRelationOperator::LessEq: return    "LessEq    <=";
        case ast::BinaryRelationOperator::Greater: return   "Greater   >";
        case ast::BinaryRelationOperator::GreaterEq: return "GreaterEq >=";
        case ast::BinaryRelationOperator::NotEqual: return  "NotEqual  /=";
    }
    return "<error>";
}

EXPLORER(BinaryRelation, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->operands.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("s" + to_string(i), " operands[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 's';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->operands[index];
})

static string to_string(ast::Sum::SumOperator op) {
    switch (op) {
        case ast::Sum::SumOperator::Plus: return  "Plus  +";
        case ast::Sum::SumOperator::Minus: return "Minus -";
    }
    return "<error>";
}

EXPLORER(Sum, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->terms.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("t" + to_string(i),     "    terms[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 't';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->terms[index];
})

static string to_string(ast::Term::TermOperator op) {
    switch (op) {
        case ast::Term::TermOperator::Times: return  "Times  *";
        case ast::Term::TermOperator::Divide: return "Divide /";
    }
    return "<error>";
}

EXPLORER(Term, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->unaries.size();
    for (int i = 0; i < n; ++i) {
        res.emplace_back("u" + to_string(i),     "  unaries[" + to_string(i) + "]");
        if (i != n - 1)
            res.emplace_back("o" + to_string(i), "operators[" + to_string(i) +
                             "] (is " + to_string(node->operators[i]) + ")");
    }
    return res;
},
{  // Action
    bool operand = command[0] == 'u';
    command.erase(0, 1);
    int index = StrToInt(command);
    if (operand) {
        output << to_string(node->operators[index]);
        return {};
    } else return node->unaries[index];
})

static string to_string(ast::PrefixOperator::PrefixOperatorKind op) {
    switch (op) {
        case ast::PrefixOperator::PrefixOperatorKind::Plus: return  "Plus  +";
        case ast::PrefixOperator::PrefixOperatorKind::Minus: return "Minus -";
        case ast::PrefixOperator::PrefixOperatorKind::Not: return   "Not";
    }
    return "<error>";
}

EXPLORER(Unary, {  // GetActionCommands
    vector<ActionCommand> res;
    int i = 0;
    for (const auto& pref : node->prefixOps) {
        res.emplace_back("e" + to_string(i), "prefixOperators[" + to_string(i) +
                         "] (is " + to_string(pref->kind) + ")");
        ++i;
    }
    res.emplace_back("p", "The primary expression");
    int n = node->postfixOps.size();
    for (int i = 0; i < n; ++i) res.emplace_back("o" + to_string(i), "postfixOperators[" + to_string(i) + "]");
    return res;
},
{  // Action
    char kind = command[0];
    command.erase(0, 1);
    int index = command.empty() ? 0 : StrToInt(command);
    switch (kind) {
        case 'e': return node->prefixOps[index];
        case 'o': return node->postfixOps[index];
        default: return node->expr;
    }
})

EXPLORER(PrefixOperator, {  // GetActionCommands
    return vector<ActionCommand>({ { "o", "The precedence of the operator" } });
},
{  // Action
    output << node->precedence();
    return {};
})

static string to_string(ast::TypeId id) {
    switch (id) {
        case ast::TypeId::String: return "String";
        case ast::TypeId::Bool: return "Bool";
        case ast::TypeId::Int: return "Int";
        case ast::TypeId::Real: return "Real";
        case ast::TypeId::None: return "None";
        case ast::TypeId::Func: return "Func";
        case ast::TypeId::Tuple: return "Tuple";
        case ast::TypeId::List: return "List";
    }
    return "<error>";
}

EXPLORER(TypecheckOperator, {  // GetActionCommands
    return vector<ActionCommand>({ { "o", "The precedence of the typecheck operator (is " +
        to_string(node->precedence()) + ")" }, {"t", "TypeID" } });
},
{  // Action
    if (command == "t")
        output << to_string(node->typeId);
    else
        output << node->precedence();
    return {};
})

EXPLORER(Call, {  // GetActionCommands
    vector<ActionCommand> res({ { "o", "The precedence of the call operator (is " +
        to_string(node->precedence()) + ")" } });
    int n = node->args.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "args[" + to_string(i) + "]");
    return res;
},
{  // Action
    if (command == "o") {
        output << node->precedence();
        return {};
    }
    return node->args[StrToInt(command)];
})

EXPLORER(AccessorOperator, {  // GetActionCommands
    return vector<ActionCommand>({ { "o", "The precedence of the accessor operator (is " +
        to_string(node->precedence()) + ")" }, { "a", "The accessor" } });
},
{  // Action
    if (command == "o") {
        output << node->precedence();
        return {};
    }
    return node->accessor;
})

EXPLORER(PrimaryIdent, {  // GetActionCommands
    return vector<ActionCommand>({ { "i", "The identifier" } });
},
{  // Action
    output << node->name->identifier;
    return {};
})

EXPLORER(ParenthesesExpression, {  // GetActionCommands
    return vector<ActionCommand>({ { "e", "The expression" } });
},
{  // Action
    return node->expr;
})

EXPLORER(TupleLiteralElement, {  // GetActionCommands
    vector<ActionCommand> res;
    if (node->ident.has_value()) res.emplace_back("n", "The name identifier");
    res.emplace_back("e", "The item expression");
    res.emplace_back("v", "The item expression");
    return res;
},
{  // Action
    if (command == "n") {
        output << node->ident.value()->identifier;
        return {};
    } else return node->expression;
})

EXPLORER(TupleLiteral, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->elements.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "elements[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->elements[StrToInt(command)];
})

EXPLORER(ShortFuncBody, {  // GetActionCommands
    return vector<ActionCommand>({ { "e", "Return expression" }, { "r", "Return expression" } });
},
{  // Action
    return node->expressionToReturn;
})

EXPLORER(LongFuncBody, {  // GetActionCommands
    return vector<ActionCommand>({ { "b", "The function body" } });
},
{  // Action
    return node->funcBody;
})

EXPLORER(FuncLiteral, {  // GetActionCommands
    vector<ActionCommand> res({ { "b", "The function body" } });
    int n = node->parameters.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "parameters[" + to_string(i) + "]");
    return res;
},
{  // Action
    if (command == "b") return node->funcBody;
    output << node->parameters[StrToInt(command)]->identifier;
    return {};
})

static string to_string(ast::TokenLiteral::TokenLiteralKind tk) {
    switch (tk) {
        case ast::TokenLiteral::TokenLiteralKind::String: return "String";
        case ast::TokenLiteral::TokenLiteralKind::Int: return "Int";
        case ast::TokenLiteral::TokenLiteralKind::Real: return "Real";
        case ast::TokenLiteral::TokenLiteralKind::True: return "True";
        case ast::TokenLiteral::TokenLiteralKind::False: return "False";
        case ast::TokenLiteral::TokenLiteralKind::None: return "None";
    }
    return "<error>";
}

EXPLORER(TokenLiteral, {  // GetActionCommands
    string kind = "Token literal kind (is " + to_string(node->kind) + ")";
    return vector<ActionCommand>({ { "t", kind }, { "k", kind }, { "v", "The literal value" } });
},
{  // Action
    if (command == "v") {
        switch (node->kind) {
        case ast::TokenLiteral::TokenLiteralKind::String:
            output << dynamic_pointer_cast<StringLiteral>(node->token)->value;
            break;
        case ast::TokenLiteral::TokenLiteralKind::Int: 
            output << dynamic_pointer_cast<IntegerToken>(node->token)->value;
            break;
        case ast::TokenLiteral::TokenLiteralKind::Real: 
            output << dynamic_pointer_cast<RealToken>(node->token)->value;
            break;
        case ast::TokenLiteral::TokenLiteralKind::True: output << "true"; break;
        case ast::TokenLiteral::TokenLiteralKind::False: output << "false"; break;
        case ast::TokenLiteral::TokenLiteralKind::None: output << "none"; break;
        }
    } else output << to_string(node->kind);
    return {};
})

EXPLORER(ArrayLiteral, {  // GetActionCommands
    vector<ActionCommand> res;
    int n = node->items.size();
    for (int i = 0; i < n; ++i) res.emplace_back(to_string(i), "items[" + to_string(i) + "]");
    return res;
},
{  // Action
    return node->items[StrToInt(command)];
})


shared_ptr<const ASTExplorer> ASTExplorerVisitor::MakeExplorer() { return this->explorer; }

#define VISITOR_HELP(classname, literalExplorer)\
void ASTExplorerVisitor::Visit##classname(ast::classname& node) {\
    this->explorer = make_shared<classname##literalExplorer>(make_shared<ast::classname>(node));\
}

#define VISITOR(classname) VISITOR_HELP(classname, Explorer)

VISITOR(Body)
VISITOR(VarStatement)
VISITOR(IfStatement)
VISITOR(ShortIfStatement)
VISITOR(WhileStatement)
VISITOR(ForStatement)
VISITOR(LoopStatement)
VISITOR(ExitStatement)
VISITOR(AssignStatement)
VISITOR(PrintStatement)
VISITOR(ReturnStatement)
VISITOR(ExpressionStatement)
VISITOR(EmptyStatement)
VISITOR(CommaExpressions)
VISITOR(CommaIdents)
VISITOR(IdentMemberAccessor)
VISITOR(IntLiteralMemberAccessor)
VISITOR(ParenMemberAccessor)
VISITOR(IndexAccessor)
VISITOR(Reference)
VISITOR(Expression)
VISITOR(OrOperator)
VISITOR(AndOperator)
VISITOR(BinaryRelation)
VISITOR(Sum)
VISITOR(Term)
VISITOR(Unary)
VISITOR(PrefixOperator)
VISITOR(TypecheckOperator)
VISITOR(Call)
VISITOR(AccessorOperator)
VISITOR(PrimaryIdent)
VISITOR(ParenthesesExpression)
VISITOR(TupleLiteralElement)
VISITOR(TupleLiteral)
VISITOR(ShortFuncBody)
VISITOR(LongFuncBody)
VISITOR(FuncLiteral)
VISITOR(TokenLiteral)
VISITOR(ArrayLiteral)

class ExplorerIO {
    shared_ptr<ast::ASTNode> rootNode;
public:
    ExplorerIO(const shared_ptr<ast::ASTNode>& root);
    static void PrintPrompt(const ASTExplorer& explorer);
    void Explore(ostream& output, istream& input);
};

