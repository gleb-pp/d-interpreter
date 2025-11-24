#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "interp/runner.h"
#include "interp/runtimeContext.h"
#include "lexer.h"
#include "locators/CodeFile.h"
#include "semantic.h"
#include "syntax.h"
#include "syntaxExplorer.h"
#include "tokenTypeStrings.h"
using namespace std;

class Options {
public:
    bool Lexer = false;
    bool Syntaxer = false;
    bool Semantics = false;
    bool Help = false;
    bool Check = false;
    bool Examples = false;
    bool NoContext = false;
    bool NoTraceback = false;
    size_t CallStackCap = 1024;
    size_t TraceLen = 50;
    optional<bool*> GetLongFlag(string name) {
        if (name == "lexer") return &Lexer;
        if (name == "syntaxer") return &Syntaxer;
        if (name == "semantics") return &Semantics;
        if (name == "help") return &Help;
        if (name == "check") return &Check;
        if (name == "examples") return &Examples;
        if (name == "nocontext") return &NoContext;
        return {};
    }
    optional<bool*> GetShortFlag(char name) {
        switch (name) {
            case 'l':
                return &Lexer;
            case 's':
                return &Syntaxer;
            case 'S':
                return &Semantics;
            case 'h':
                return &Help;
            case 'c':
                return &Check;
            case 'C':
                return &NoContext;
            case 'T':
                return &NoTraceback;
            default:
                return {};
        }
    }
    bool SetLongFlag(string name) {
        auto flag = GetLongFlag(name);
        if (!flag.has_value()) return false;
        **flag = true;
        return true;
    }
    bool SetShortFlag(char name) {
        auto flag = GetShortFlag(name);
        if (!flag.has_value()) return false;
        **flag = true;
        return true;
    }
    static constexpr const char* HELP =
        R"%%(dinterp - an interpreter for the D language.

Usage: dinterp [OPTIONS] [--] [file1.d file2.d ...]

Options:
    --help       -h  Show this text.
    --check      -c  Only check for errors, do not run.
    --examples       Show some usage examples.
    --lexer      -l  Stop after lexical analysis, output the tokens.
    --syntaxer   -s  Stop after syntactic analysis, start interactive AST traversal.
    --semantics  -S  Stop after semantic analysis, start interactive AST traversal.
    --nocontext  -C  Do not show code excerpts below errors.
    --notrace    -T  Do not show the call stack traceback on error.
Parameter options:
    --callstack <nonnegative integer>  Set the call stack capacity (default = 1024).
    --tracelen  <nonnegative integer>  On error, output at most this many call stack entries (default = 50).

Every argument after -- is assumed to be a file name.
)%%";
    static constexpr const char* EXAMPLES =
        R"%%(-- EXAMPLES --

Run a script:
dinterp script.d

Tokenize files:
dinterp -l abc.d program.d

Check programs for errors:
dinterp -c *.d

Check programs for lexical errors:
dinterp *.d -lc

Run a program named -abc.d:
dinterp -- -abc.d

Explore the syntax of a program:
dinterp -s prog.d

Explore the optimized syntax of a program:
dinterp -S prog.d
)%%";
};

static optional<size_t> ParseSizeT(const std::string& s) {
    size_t res = 0;
    for (char ch : s) {
        if (ch < '0' || ch > '9') return {};
        size_t prevres = res;
        res = res * 10 + (ch - '0');
        if ((res - (ch - '0')) / 10 != prevres) return {};
    }
    return res;
}

bool InterpretArgs(int argc, char** argv, Options& opts, vector<string>& files) {
    bool onlyFiles = false;
    for (int i = 1; i < argc; i++) {
        if (onlyFiles) {
            files.emplace_back(argv[i]);
            continue;
        }
        string arg = argv[i];
        if (arg.starts_with("--")) {
            arg.erase(0, 2);
            if (arg.empty()) {
                onlyFiles = true;
                continue;
            }
            if (arg == "tracelen" || arg == "callstack") {
                ++i;
                if (i == argc) {
                    cerr << "Expected a number after \"--" << arg << "\"\n";
                    return false;
                }
                optional<size_t> parsedarg = ParseSizeT(argv[i]);
                if (!parsedarg) {
                    cerr << "Could not parse a nonnegative integer: \"" << argv[i] << "\"\n";
                    return false;
                }
                if (arg == "tracelen")
                    opts.TraceLen = *parsedarg;
                else
                    opts.CallStackCap = *parsedarg;
                continue;
            }
            if (!opts.SetLongFlag(arg)) {
                cerr << "Unknown flag: --" << arg << '\n';
                return false;
            }
            continue;
        }
        if (arg.starts_with("-")) {
            arg.erase(0, 1);
            bool fail = false;
            for (char ch : arg) {
                if (!opts.SetShortFlag(ch)) {
                    fail = true;
                    cerr << "Unknown flag: -" << ch << '\n';
                }
            }
            if (fail) return false;
            continue;
        }
        files.push_back(arg);
    }
    return true;
}

void PrintTokens(const locators::CodeFile& file, const vector<shared_ptr<Token>>& tokens) {
    cout << file.FileName() << '\n';
    size_t padding = 0;
    for (auto& ptoken : tokens) padding = max(padding, TokenTypeToString(ptoken->type).length());
    padding += 2;
    for (auto& ptoken : tokens) {
        auto typestr = TokenTypeToString(ptoken->type);
        cout << typestr << string(padding - typestr.size(), ' ')
             << file.AllText().substr(ptoken->span.position, ptoken->span.length) << '\n';
    }
    cout << "Total: " << tokens.size() << " tokens\n\n";
}

class SpyCompilationLog : public complog::ICompilationLog {
    bool logged = false;
    complog::ICompilationLog& dest;

public:
    SpyCompilationLog(complog::ICompilationLog& dest) : dest(dest) {}
    void ResetTrigger() { logged = false; }
    bool SomethingLogged() const { return logged; }
    void Log(const std::shared_ptr<complog::CompilationMessage>& message) override {
        logged = true;
        dest.Log(message);
    }
    virtual ~SpyCompilationLog() override = default;
};

static void WaitForUserToReadMessages() {
    cerr << "\nSome messages were printed above. Press Enter when you are ready...";
    string buf;
    getline(cin, buf);
}

bool ProcessFile(string filename, const Options& opts, complog::ICompilationLog& log) {
    SpyCompilationLog slog(log);
    shared_ptr<const locators::CodeFile> file;
    {
        ifstream in(filename);
        if (!in) {
            perror(("Cannot open " + filename).c_str());
            return false;
        }
        stringstream sstr;
        sstr << in.rdbuf();
        in.close();
        file = make_shared<locators::CodeFile>(filename, sstr.str());
    }
    auto maybeTokens = Lexer::tokenize(file, slog);
    if (!maybeTokens.has_value()) {
        cerr << "A lexical error was encountered in " << filename << ", stopping.\n";
        return false;
    }
    auto& tokens = maybeTokens.value();
    if (opts.Lexer) {
        if (!opts.Check) PrintTokens(*file, tokens);
        return true;
    }
    auto maybeProg = SyntaxAnalyzer::analyze(tokens, file, slog);
    if (!maybeProg.has_value()) {
        cerr << "A syntax error was encountered in " << filename << ", stopping.\n";
        return false;
    }
    auto& prog = maybeProg.value();
    if (opts.Syntaxer) {
        if (slog.SomethingLogged()) WaitForUserToReadMessages();
        if (!opts.Check) {
            ExplorerIO io(prog);
            io.Explore(cout, cin);
        }
        return true;
    }
    if (!semantic::Analyze(slog, prog)) {
        cerr << "A semantic error was encountered in " << filename << ", stopping.\n";
        return false;
    }
    if (opts.Semantics) {
        if (slog.SomethingLogged()) WaitForUserToReadMessages();
        if (!opts.Check) {
            ExplorerIO io(prog);
            io.Explore(cout, cin);
        }
        return true;
    }

    if (opts.Check) return true;
    interp::RuntimeContext context(cin, cout, opts.CallStackCap, opts.TraceLen);
    interp::Run(context, *prog);
    if (context.State.IsThrowing()) {
        cout.flush();
        auto& details = context.State.GetError();
        cerr << "Runtime error encountered while executing " << filename << ".\n\n";
        if (!opts.NoTraceback) {
            cerr << "Call stack traceback (most recent call LAST):\n";
            details.StackTrace.WriteToStream(cerr);
            cerr << "\n\n";
        }
        cerr << "At " << details.Position.Pretty() << ":\n";
        if (!opts.NoContext) details.Position.WritePrettyExcerpt(cerr, 100);
        cerr << details.Error.what() << '\n';
        cerr.flush();
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    vector<string> files;
    Options opts;
    if (!InterpretArgs(argc, argv, opts, files)) return 1;
    bool doneSomething = false;
    if (opts.Help) {
        cout << Options::HELP << endl;
        doneSomething = true;
    }
    if (opts.Examples) {
        cout << Options::EXAMPLES << endl;
        doneSomething = true;
    }
    bool failed = false;
    if (files.size()) doneSomething = true;

    complog::CompilationMessage::FormatOptions format = complog::CompilationMessage::FormatOptions::All(80);
    if (opts.NoContext) format = format.WithoutContext();
    complog::StreamingCompilationLog log(cerr, format);

    for (auto filename : files) {
        if (!ProcessFile(filename, opts, log)) failed = true;
    }

    if (!doneSomething) {
        cerr << "Nothing to do. Type 'dinterp -h' for help.\n";
    }

    return static_cast<int>(failed);
}
