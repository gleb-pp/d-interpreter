#include "fixture.h"

#include <sstream>

#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/interp/runner.h"
#include "dinterp/interp/runtimeContext.h"
#include "dinterp/lexer.h"
#include "dinterp/semantic.h"
#include "dinterp/syntax.h"
#include "fstream"
#include "sstream"
using namespace std;
using namespace dinterp;
using namespace interp;

void Sample::ReadFile(const char* name, bool compiles) {
    ifstream fs(name);
    ASSERT_TRUE(fs);
    stringstream ss;
    ss << fs.rdbuf();
    filename = name;
    file = make_shared<locators::CodeFile>(name, ss.str());
    auto opttks = Lexer::tokenize(file, log);
    if (!opttks) {
        if (!compiles) return;
        log.WriteToStream(cerr, complog::CompilationMessage::FormatOptions::All(100));
        FAIL() << "Lexer error\n";
    }
    auto optprog = SyntaxAnalyzer::analyze(*opttks, file, log);
    if (!optprog) {
        if (!compiles) return;
        log.WriteToStream(cerr, complog::CompilationMessage::FormatOptions::All(100));
        FAIL() << "Syntax error\n";
    }
    program = *optprog;
    if (!semantic::Analyze(log, program)) {
        if (!compiles) return;
        log.WriteToStream(cerr, complog::CompilationMessage::FormatOptions::All(100));
        FAIL() << "Semantic error\n";
    }
    ASSERT_TRUE(compiles) << "Expected to fail\n";
}

void Sample::RunAndExpect(const char* input, const char* output) {
    istringstream sin(input);
    ostringstream sout;
    RuntimeContext context(sin, sout, 1000, 10);
    interp::Run(context, *program);
    if (context.State.IsThrowing()) {
        auto& detail = context.State.GetError();
        detail.StackTrace.WriteToStream(cerr);
        cerr << "\n\n";
        detail.Position.WritePrettyExcerpt(cerr, 100);
        FAIL() << "Runtime error: " << detail.Error.what() << '\n';
    }
    ASSERT_EQ(sout.str(), output);
}

void Sample::RunAndExpectCrash(const char* input) {
    istringstream sin(input);
    ostringstream sout;
    RuntimeContext context(sin, sout, 1000, 10);
    interp::Run(context, *program);
    ASSERT_TRUE(context.State.IsThrowing());
}
