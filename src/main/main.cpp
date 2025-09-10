#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include "complog/CompilationLog.h"
#include "lexer.h"
#include "locators/CodeFile.h"
using namespace std;

class Options {
public:
    bool Lexer = false;
    bool Help = false;
    bool Check = false;
    optional<bool*> GetLongFlag(string name) {
        if (name == "lexer") return &Lexer;
        if (name == "help") return &Help;
        if (name == "check") return &Check;
        return {};
    }
    optional<bool*> GetShortFlag(char name) {
        switch (name) {
            case 'l': return &Lexer;
            case 'h': return &Help;
            case 'c': return &Check;
            default: return {};
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
    --help      -h  Show this text.
    --check     -c  Only check for errors, do not run.
    --examples      Show some usage examples.
    --lexer     -L  Stop after lexical analysis, output the tokens.
    --locators  -l  Show code excerpts below errors.
    --

Every argument after -- is assumed to be a file name.
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

int main(int argc, char** argv) {
    vector<string> files;
    Options opts;
    if (!InterpretArgs(argc, argv, opts, files)) return 1;
    bool doneSomething = false;
    if (opts.Help) {
        cout << Options::HELP << endl;
        doneSomething = true;
    }
    bool failed = false;
    if (files.size()) doneSomething = true;

    for (auto filename : files) {
        string content;
        {
            ifstream in(filename);
            if (!in) {
                perror(("Cannot open " + filename).c_str());
                failed = true;
                continue;
            }
            stringstream sstr;
            sstr << in.rdbuf();  // read everything
            in.close();
            content = sstr.str();
        }
        shared_ptr<locators::CodeFile> file = make_shared<locators::CodeFile>(filename, content);

        complog::StreamingCompilationLog log(cerr, );
        vector<shared_ptr<Token>> tokens = Lexer::tokenize(file, log);
    }
}
