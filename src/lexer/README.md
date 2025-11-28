# Lexer

The purpose of this library is to split the program text into a list of tokens. Thus, the only function that needs be
called is `Lexer::Tokenize`.

This library provides the following classes:

- `Token` is the base class for all tokens, it contains the type of its token and the position within the file;
    - `Type` is an enumeration that has many values, see the header for a full list;
- `IdentifierToken` is a token of type `tkIdent`, contains its string value;
- `IntegerToken` is a token of type `tkIntLiteral`, contains its value as a `BigInt`;
- `RealToken` is a token of type `tkRealLiteral`, contains its value as a `long double`;
- `StringLiteral` is a token of type `tkStringLiteral`, contains the encoded value as a `std::string`.

Four diagnostics can be produced by the lexer:

- `LexerError`
- `NewlineInStringLiteralError`
- `WrongEscapeSequenceError`
- `UnclosedStringLiteralError`

The class `Lexer` only contains the static method `Tokenize`. This method may return a list of tokens or nothing in case
of an error. Errors are reported to the logger supplied as an argument.
