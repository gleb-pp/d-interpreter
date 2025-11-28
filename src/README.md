# Implementation Overview

The interpreter implementation is divided into subsystems:

- `locators` library provides abstractions of a code file and positions within it;
- `complog` library provides a way to produce diagnostic messages;
- `lexer` parses text into **D** language tokens;
- `syntax` builds abstract syntax trees (ASTs);
- `bigint` implements huge integers;
- `runtime` implements the **D** language type/value interactions;
- `semantics` library (contained in the `semantic` directory) performs checks, optimizations, and some mandatory
transformations;
- `interp` library runs the checked code;
- `main` directory contains the code for the `dinterp` binary.

In the end, all libraries get combined into a single `dinterptools` library that is then used in the interpreter
program.

To see a detailed description of a subsystem, see the README file in the corresponding directory.

# Interpretation steps

| Stage          | Libraries                        | Product                       |
| -------------- | -------------------------------- | ----------------------------- |
| Opening file   | `locators`                       | Code files                    |
| Lexing         | `lexer`                          | List of tokens                |
| Syntaxing      | `syntax`                         | An Abstract Syntax Tree (AST) |
| Semantics      | `semantics`, `runtime`, `bigint` | A modified AST                |
| Interpretation | `interp`, `runtime`, `bigint`    | Program execution             |

`complog` is used on all stages except opening the file.

### What is `debugbuild`

This directory contains settings for running the [codelldb](https://github.com/vadimcn/vscode-lldb) debugger with Neovim
using the plugins [Overseer](https://github.com/stevearc/overseer.nvim),
[nvim-dap](https://github.com/mfussenegger/nvim-dap), and [nvim-dap-ui](https://github.com/rcarriga/nvim-dap-ui).
