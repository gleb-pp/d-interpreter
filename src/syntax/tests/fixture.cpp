#include "fixture.h"

#include <fstream>
#include <sstream>

#include "complog/CompilationLog.h"
#include "gtest/gtest.h"
#include "lexer.h"
#include "syntax.h"
using namespace std;

void FileSample::ReadFile(std::string name, bool expectSuccess) {
    ifstream fs(name);
    ASSERT_TRUE(fs);
    stringstream text;
    text << fs.rdbuf();
    file = make_shared<locators::CodeFile>(name, text.str());
    log = make_unique<complog::AccumulatedCompilationLog>();
    auto optTokens = Lexer::tokenize(file, *log);
    log->WriteToStream(cout, complog::Severity::Error(), complog::CompilationMessage::FormatOptions::All(80));
    ASSERT_TRUE(optTokens.has_value());
    tokens = optTokens.value();
    log.reset();

    log = make_unique<complog::AccumulatedCompilationLog>();
    auto optProgram = SyntaxAnalyzer::analyze(tokens, file, *log);
    if (expectSuccess) {
        log->WriteToStream(cout, complog::Severity::Error(), complog::CompilationMessage::FormatOptions::All(80));
        ASSERT_TRUE(optProgram.has_value());
    }
    program = *optProgram;
}

void FileSample::ExpectFailure(size_t line, size_t col) {
    for (auto& msg : log->Messages()) {
        if (msg->MessageSeverity() == complog::Severity::Error()) {
            for (auto& loc : msg->Locators())
                if (loc.LineCol() == make_pair(line, col)) return;
        }
    }
    cout << "The test was expected to fail.\nFull compilation log:\n";
    log->WriteToStream(cout, complog::CompilationMessage::FormatOptions::All(80));
    ASSERT_FALSE(true) << "The compilation was expected to fail, but it passed.";
}
