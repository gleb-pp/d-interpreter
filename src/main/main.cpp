#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "lexer.h"
#include "locators/CodeFile.h"
#include "tokenTypeStrings.h"
using namespace std;

class Options {
public:
    bool Lexer = false;
    bool Help = false;
    bool Check = false;
    bool Examples = false;
    bool NoContext = false;
    optional<bool*> GetLongFlag(string name) {
        if (name == "lexer") return &Lexer;
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
            case 'h':
                return &Help;
            case 'c':
                return &Check;
            case 'C':
                return &NoContext;
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
    --nocontext  -C  Do not show code excerpts below errors.

Every argument after -- is assumed to be a file name.
)%%";
    static constexpr const char* EXAMPLES =
        R"%%(-- EXAMPLES --

Tokenize files:
dinterp -l abc.d program.d

Check programs for errors:
dinterp -c *.d

Check programs for lexical errors:
dinterp *.d -lc

Run a program named -abc.d:
dinterp -- -abc.d
)%%";
};

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

bool ProcessFile(string filename, const Options& opts, complog::ICompilationLog& log) {
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
    auto maybeTokens = Lexer::tokenize(file, log);
    if (!maybeTokens.has_value()) {
        cerr << "A lexical error was encountered in " << filename << ", stopping.\n";
        return false;
    }
    auto& tokens = maybeTokens.value();
    // Currently, show tokens even if --lexer is not set
    if (!opts.Check) PrintTokens(*file, tokens);
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

    for (auto filename : files)
        if (!ProcessFile(filename, opts, log)) failed = true;

    if (!doneSomething) {
        cerr << "Nothing to do. Type 'dinterp -h' for help.\n";
    }

    return static_cast<int>(failed);
}
