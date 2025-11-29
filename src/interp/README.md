# Interp

This library implements code execution directly from the modified syntax tree. To run a **D** program, it is sufficient
to call `dinterp::interp::Run`.

The library provides 1 custom abstract subclass of `RuntimeValue` and 2 non-abstract ones:

- `UserCallable` is the base class for all functions that require a `RuntimeContext` to be called (see below):
    - `InputFunction` accepts no arguments and returns a line read from the input stream (`RuntimeContext::Input`)
    without the line feed character;
    - `Closure` is a user-defined function that captures zero or more *variables* (not their values) from external
    scopes. When called, the closure starts a new scope stack that only contains captured variables and arguments,
    initially.

The library introduces the following types:

- `Variable` is a named container for a `RuntimeValue`;
- `Scope` is an associative array from strings (variable names) to variables;
- `ScopeStack` is a stack of `Scope`s that also implements lookups (topmost scopes overshadow lower ones), declarations,
and assignments (these are passed to the topmost scope);
- `CallStackTrace` is a list of locators where function calls took place, possibly with several entries skipped in the
middle;
- `CallStack` is a stack of locators where function calls took place, used to track recursion depth and create
`CallStackTrace`s;
- `RuntimeState` is an algebraic data type (a smart enum) that can have values:
    - `Running` (normal state),
    - `Exiting` (`exit` encountered, terminating execution until a reaching a cycle),
    - `Returning(RuntimeValue)` (`return` encountered, terminating execution until exiting from a closure),
    - `Throwing(the error, position, stack trace)` (an error encountered, terminating execution);
- `RuntimeContext` is an object that holds the input/output streams, the current execution state (`RuntimeState`), and
the call stack (`CallStack`). It also stores the settings of maximum call stack capacity and desired length of the stack
trace to report in case of an error.
- `UnaryOpExecutor` is a visitor that evaluates an `Unary` AST node;
- `Executor` is a visitor that evaluates expressions and executes statements.

## Known issues

If the input stream is closed or is in an error state, `InputFunction` (`input()`) immediately returns an empty string.
