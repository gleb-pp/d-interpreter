The interpreter for the [dynamic language](https://github.com/gleb-pp/d-interpreter/blob/main/proj-describtion/Project%20D.pdf) implemented in С++.

# Lexical Analyser

**Lexical Analyser** splits the initial code into self-contained tokens.

#### Input for the Lexical Analyser
```
var x := 5
print x
```
#### Output for the Lexical Analyser
```
tkVar tkIdent(x) tkAssign tkIntLiteral(5) tkNewLine tkPrint tkIdent(x)
```

#### Proccessing
**Token** class contains all the information about the tokens:
- span (position and lenght) to indicate errors
- type to process the code futher (by syntax analyser)
- typeChars (list of pairs `<">=", "tkGreaterEq">`) to match the substrings and token types

**Token** class has 4 subcalsses with additional information:
- **Integer** with an attirbute `value`
- **Real** with an attirbute `value`
- **StringLiteral** with an attirbute `value`
- **Identifier** with an attirbute `identifier` (e.g. the name of the variable)

**Lexer** class has the `tokenize()` function that process the lexical analysis:
- **Lexer** checks if it met the code comments (`//` and something until the end of the line)
- **Lexer** checks if it met the string (some characters between two `"` symbols)
- **Lexer** checks if it met the number (some integer `d` or real value `d.d`)
- **Lexer** checks if it met the token (some keyword of the language)
- **Lexer** checks if it met the identrifier (some latin letters / digits / underscores)