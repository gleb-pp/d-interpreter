# Syntax

The purpose of this library is to build the syntax tree of a program that is represented as a list of tokens. Thus, only
the `SyntaxAnalyzer::analyze` method needs be called.

The library provides many classes related to various syntactic features of the **D** language:

- `ASTNode` is the base class for all syntax tree nodes, contains the `SpanLocator` position of the node in the source;
- `Statement` is the base class for all statements;
    - `Body` is a statement that contains a list of other statements, executed one after another in a new scope;
    - `VarStatement` declares and possibly initializes new variables;
    - `IfStatement` is used to conditionally execute a body;
    - `ShortIfStatement` is used to conditionally execute a single statement;
    - `WhileStatement` makes a cycle that, when the condition is true, executes a body and checks again;
    - `ForStatement` iterates (with or without introducing a new variable) over a range or a collection;
    - `LoopStatement` makes an endless cycle;
    - `ExitStatement` breaks out of a cycle;
    - `AssignStatement` assigns a value to a variable or an element of a collection (array or tuple);
    - `PrintStatement` outputs values to console, without inserting separators or line breaks; a `print` without
    arguments is allowed, but does nothing;
    - `ReturnStatement` returns a value from a function, immediately exiting from the function;
    - `ExpressionStatement` evaluates an expression that may have side effects;
- `CommaExpressions` is a list of **one or more** expressions separated by commas;
- `CommaIdents` is a list of **one or more** identifier separated by commas;
- `Accessor` is the base class for all member-accessing constructs;
    - `IdentMemberAccessor` is accessing a field of an object (e.g. `.Field`);
    - `IntLiteralMemberAccessor` is accessing a field of a tuple by index (e.g. `.3`);
    - `ParenMemberAccessor` is accessing a field of a tuple by an evaluated index (e.g. `.(1 + 2)`);
    - `IndexAccessor` is accessing an array element (e.g. `array[i]`);
- `Reference` is intended to be assigned to; it is an identifier with a chain of accessors after it;
- `Expression` is the base class for all that can be evaluated into a value;
    - `XorOperator` is the logical exclusive disjunction operator on several operands;
    - `OrOperator` is the logical disjunction operator on several operands;
    - `AndOperator` is the logical conjunction operator on several operands;
    - `BinaryRelation` is a conjunction of comparisons;
    - `Sum` is a sequence of terms separated by pluses and minuses;
    - `Term` is a sequence of unary blocks (`Unary` nodes) separated by multiplications and divisions;
    - `UnaryNot` represents a negated logical expression;
    - `Unary` is a block of unary prefix and suffix operators surrounding a `Primary`;
    - `PrefixOperator` is an unary plus or minus;
    - `PostfixOperator` is the base class for accessors, calls, and typechecks:
        - `TypecheckOperator` checks if an object is of a certain type: `int`, `real`, `string`, `bool`, `none`, `func`,
        `{}` (tuple), or `[]` (array).
        - `Call` contains arguments that must be passed in a call to the object (usually the object is a function);
        - `AccessorOperator` contains an `Accessor` to apply to the current object;
    - `Primary` is the base class for all values that are used as operands to unary and binary operators:
        - `PrimaryIdent` is a variable name;
        - `ParenthesesExpression` contains a nested expression, enclosed in parentheses;
        - `TupleLiteral` expresses a tuple (a collection of other, possibly named, objects);
        - `FuncLiteral` defines a closure (a function that may capture external variables);
        - `TokenLiteral` is a literal that fits within one token: a string literal, integer literal, real literal,
        `true`, `false`, or `none`;
        - `ArrayLiteral` expresses an array of values that are given consecutive indices starting from $1$;
    - `TupleLiteralElement` contains an expression to evaluate and optionally a name for it;
    - `FuncBody` is the base class for `FuncLiteral` code:
        - `ShortFuncBody` is an expression to return;
        - `LongFuncBody` is a body of statements to execute sequentially and expect a `return`.

Auxiliary classes are:

- `EmptyVarStatement` is an error diagnostic about having an empty `var` statement;
- `UnexpectedTokenTypeError` is a generic syntax error diagnostic;
- `WrongNumberOfOperatorsSupplied` is a runtime error that is thrown when an operator node like `Sum` gets constructed
with the number of operators not being one less than the number of operands;
- `SyntaxErrorReport` is a utility class that builds `UnexpectedTokenTypeError`s;
- `TokenScanner` is a utility class that provides convenient methods to read tokens;
    - `TokenScanner::AutoBlock` is a scoped guard which is meant to be used like an `std::lock_guard`: it starts a
    scanning block in its constructor and ends it in the destructor;
- `IASTVisitor` is a base interface for implementing the Visitor pattern on AST nodes;
- `SyntaxAnalyzer` only provides a static method `analyze`.

## Operator precedence

In expressions:

| Operator         | Precedence   |
| ---------------- | ------------ |
| `XorOperator`    | 6 (latest)   |
| `OrOperator`     | 5            |
| `AndOperator`    | 4            |
| `UnaryNot`       | 3            |
| `BinaryRelation` | 2            |
| `Sum`            | 1            |
| `Term`           | 0 (earliest) |

In unary blocks (`Unary` nodes):

| Operator                    | Precedence   |
| --------------------------- | ------------ |
| `TypecheckOperator`         | 3 (latest)   |
| `PrefixOperator` (`+`, `-`) | 2            |
| `Call`                      | 1 (earliest) |
| `AccessorOperator`          | 1 (earliest) |

## Extended Backus-Naur Form

Below is a more or less complete description of the language syntax; it has been extended (and corrected) compared to
the original description in `/proj-description/Project D.pdf`.

The notation uses `< >` and `<* *>` sections (for example, in definitions of `ParenthesesExpression` and `Body`,
respectively) to denote sections where line breaks (tkNewLine) must be ignored (`< >`) or must be considered a statement
separator (`<* *>`). Tokens - terminal symbols - start with `tk`; non-terminal symbols start with a capital letter.
Also, here, definitions are not separated by semicolons because, in the author's subjective opinion, they worsen the
readability and do not provide any value.

Expressions are parsed using the Shunting yard algorithm, so the EBNF for them are purely informational.

```text
PROGRAM -> Body

Sep -> tkSemicolon | tkNewLine

Body -> <* { Statement Sep { Sep } } *>

Statement -> VarStatement // var a := 3
    | IfStatement         // if a = 3 then ... else ...
    | ShortIfStatement    // if a = 3 => ...
    | WhileStatement      // while ... loop ... end
    | ForStatement        // for i in start..stop loop ... end
    | LoopStatement       // loop print "working..."; end
    | ExitStatement       // exit
    | AssignStatement     // a := 3
    | PrintStatement      // print a, b, "c"
    | ReturnStatement     // return a + 4
    | ExpressionStatement // myObj.method(a, a + 1)

VarStatement -> tkVar [tkNewLine] tkIdent [ AssignExpression ]
    { tkComma [tkNewLine] tkIdent [ AssignExpression ] }

AssignExpression -> tkAssign Expression

IfStatement -> tkIf < Expression > tkThen Body [ tkElse Body ] tkEnd

ShortIfStatement -> tkIf < Expression > tkArrow [tkNewLine] Statement

WhileStatement -> tkWhile < Expression > LoopBody

LoopBody -> tkLoop Body tkEnd

ForStatement -> tkFor [ tkIdent tkIn ] < Expression > [ tkRange < Expression > ] LoopBody

LoopStatement -> LoopBody

ExitStatement -> tkExit

AssignStatement -> Reference tkAssign Expression

PrintStatement -> tkPrint [ CommaExpressions ]

CommaExpressions -> Expression { tkComma Expression }

CommaIdents -> tkIdent { tkComma tkIdent }

ReturnStatement -> tkReturn [ Expression ]

ExpressionStatement -> Expression

Reference -> tkIdent { Accessor }

Accessor -> MemberAccessor | IndexAccessor

MemberAccessor -> tkDot ( tkIdent | tkIntLiteral | ParenthesesExpression )
//                        a.value   a.2            a.(1 + i)

IndexAccessor -> tkOpenBracket < Expression > tkClosedBracket

// Binary operator precedence:
// 1. * /
// 2. + -
// 3. > >= < <= = /=
// 4. not
// 5. and
// 6. or
// 7. xor
// Unary operator precedence:
// 1. function(args)  obj.field  arr[index]  // call & accessors
// 2. +num -num
// 3. obj is type

// Expressions are parsed with Shunting yard algorithm
Expression -> UnaryOrNotExpr { BinaryOperator UnaryOrNotExpr }
UnaryOrNotExpr -> tkNot Expression(without logical operators) | Unary

// Binary operators are not parsed directly, these definitions are purely informational
XorOperator -> OrOperator { tkXor OrOperator }
OrOperator -> AndOperator { tkOr AndOperator }
AndOperator -> BinaryRelation { tkAnd BinaryRelation }
BinaryRelation -> Sum { BinaryRelationOperator Sum }
BinaryRelationOperator -> tkLess | tkLessEq | tkGreater | tkGreaterEq | tkEqual | tkNotEqual
Sum -> Term { (tkPlus | tkMinus) Term }
Term -> Unary { (tkTimes | tkDivide) Unary }

Unary -> {PrefixOperator} Primary {PostfixOperator}
PrefixOperator -> tkMinus | tkPlus
PostfixOperator -> TypecheckOperator | Call | AccessorOperator

TypecheckOperator -> tkIs TypeId
TypeId -> tkInt | tkReal | tkString | tkBool | tkNone | tkFunc
    | tkOpenBracket tkClosedBracket | tkOpenCurlyBrace tkClosedCurlyBrace
Call -> tkOpenParenthesis < [ CommaExpressions ] > tkClosedParenthesis
AccessorOperator -> Accessor

Primary -> PrimaryIdent | ParenthesesExpression | FuncLiteral | TokenLiteral | ArrayLiteral | TupleLiteral
PrimaryIdent -> tkIdent
ParenthesesExpression -> tkOpenParenthesis < Expression > tkClosedParenthesis
TokenLiteral -> tkStringLiteral | tkIntLiteral | tkRealLiteral | tkTrue | tkFalse | tkNone
FuncLiteral -> tkFunc tkOpenParenthesis < [ CommaIdents ] > tkClosedParenthesis FuncBody
ArrayLiteral -> tkOpenBracket < [ CommaExpressions ] > tkClosedBracket
TupleLiteral -> tkOpenCurlyBrace < [ TupleLiteralElement { tkComma TupleLiteralElement } ] > tkClosedCurlyBrace
TupleLiteralElement -> [ tkIdent tkAssign ] Expression
FuncBody -> ShortFuncBody | LongFuncBody
ShortFuncBody -> tkArrow Expression
LongFuncBody -> tkIs Body tkEnd
```
