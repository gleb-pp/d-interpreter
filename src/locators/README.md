# Locators

This library provides 3 classes:

- `CodeFile` encapsulates a source file (its name and all of its content);
- `Locator` encapsulates a position within a file;
- `SpanLocator` encapsulates a file segment.

`CodeFile` makes it simple to work with lines and line numbers. Locators are widely used throughout the code to report
diagnostics.
