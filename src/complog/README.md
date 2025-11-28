# Compilation Logs

This library provides the base class for all diagnostic messages and a few logger classes. In particular:

- `Severity` is a smart enumeration of `Info`, `Warning`, and `Error`, can be compared;
- `CompilationMessage` is the abstract base class that encapsulates a diagnostic;
    - Method `WriteMessageToStream` must be implemented, it must only output the message text (ending with a newline);
    - Method `Locators` must be implemented to return all locators pointing to the problematic code;
    - Method `SpanLocators` can be implemented to return all **span** locators pointing to the problematic code (by
    default returns an empty vector);
    - Method `WriteToStream` formats the message nicely and also outputs the problematic code excerpts if told to;
- `CompilationMessage::FormatOptions` tell the `CompilationMessage::WriteToStream` method how to output the message;
- `ICompilationLog` is the base interface for a diagnostic logger;
- `StreamingCompilationLog` immediately outputs logged messages to a stream;
- `AccumulatedCompilationLog` stores logged messages and can output them later;
- `CombinedCompilationLog` fans out compilation messages to several loggers (implementing the Decorator design pattern).

Compilation messages and loggers are ubiquitously used in the code base.
