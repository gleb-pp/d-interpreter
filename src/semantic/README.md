# Semantics

This library implements preliminary code checking and modification. As the author, I advise against using the classes it
provides directly; instead, I recommend calling the `dinterp::semantic::Analyze` function.

The library provides 2 custom AST nodes (see `src/syntax/README.md`):

- `PrecomputedValue` is an `Expression` that contains a `runtime::RuntimeValue`
which is calculated during the analysis stage;
- `ClosureDefinition` is an `Expression` that **always** replaces `FuncLiteral`s. In addition to parameter names and
function code, `ClosureDefinition` contains the purity status of the function and a list of *captured variables* from
outside of the function scope. Closures are instantiated during execution from `ClosureDefinition`s.

The library uses the following classes internally:

- `AstDeepCopier` is a visitor that clones a syntax tree;
- `UnaryOpChecker` is a visitor that checks and modifies an `Unary`;
- `StatementChecker` is a visitor that checks and modifies statements (as opposed to expressions);
- `ValueTimeline` is an encapsulation of an uncertain program state, instances of which can be *merged* (used to
implement branching);
- `ExpressionChecker` is a visitor that checks and modifies an `Expression`.

The checker can produce the following diagnostics:

- `SpanLocatorMessage` is a convenience base class for all messages with only one `SpanLocator`;
- Errors:
    - `ConditionMustBeBoolean`;
    - `VariableNotDefined`;
    - `VariableRedefined`;
    - `OperatorNotApplicable`;
    - `IterableExpected`: in a `for` cycle without a *range* token (`..`), the expression must evaluate to an array or a
      tuple;
    - `IntegerBoundaryExpected`: in a `for` cycle with a *range* token (`..`), both range boundaries must be integers;
    - `EvaluationException`: while precomputing, an exception was encountered;
    - `NoSuchField`;
    - `CannotAssignNamedFieldInTuple`;
    - `FieldsOnlyAssignableInTuples`;
    - `CannotAssignIndexedFieldInTuple`;
    - `SubscriptAssignmentOnlyInArrays`;
    - `BadSubscriptIndexType`: expected an integer in square brackets, but the value is known to not be one;
    - `TriedToCallNonFunction`;
    - `WrongArgumentCount`;
    - `WrongArgumentType`;
    - `DuplicateFieldNames`: in a tuple literal, duplicate field names are not allowed;
    - `DuplicateParameterNames`;
    - `ExitOutsideOfCycle`;
    - `ReturnOutsideOfFunction`;
- Warnings:
    - `AssignedValueUnused`
    - `VariableNeverUsed`;
    - `NoneValueAccessed`: referencing a variable whose value is known to be `none`;
    - `CodeUnreachable`;
    - `IfConditionAlwaysKnown`;
    - `WhileConditionNotBoolAtStart`;
    - `WhileConditionFalseAtStart`: this causes the `while` cycle to be removed from the program;
    - `ExpressionStatementNoSideEffects`: this causes the expression statement to be removed from the program;
    - `IntegerZeroDivisionWarning`: explicit zero division where it is not certain if the dividend is a real value.

All of this is done in a single depth-first syntax tree traversal.

### Known issues

Unused variables are reported, but not removed.
