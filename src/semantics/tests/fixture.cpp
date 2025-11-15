#include "fixture.h"

#include <fstream>
#include <sstream>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "gtest/gtest.h"
#include "lexer.h"
#include "semantic.h"
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
    log->WriteToStream(cout, complog::Severity::Error(), complog::CompilationMessage::FormatOptions::All(80));
    ASSERT_TRUE(optProgram.has_value());
    program = optProgram.value();
    log.reset();

    log = make_unique<complog::AccumulatedCompilationLog>();
    bool ok_semantic = semantic::Analyze(*log, program);
    if (expectSuccess) {
        log->WriteToStream(cout, complog::Severity::Error(), complog::CompilationMessage::FormatOptions::All(80));
        ASSERT_TRUE(ok_semantic);
    }
}

void FileSample::ExpectFailure(size_t line, size_t col) {
    for (auto& msg : log->Messages()) {
        if (msg->MessageSeverity() == complog::Severity::Error()) {
            for (auto& loc : msg->Locators())
                if (loc.LineCol() == make_pair(line, col)) return;
        }
    }
    cout << "The test was expected to fail.\nFull semantic analysis log:\n";
    log->WriteToStream(cout, complog::CompilationMessage::FormatOptions::All(80));
    ASSERT_FALSE(true) << "The compilation was expected to fail, but it passed.";
}

void FileSample::ExpectFailure(size_t line, size_t col, string subMsg) {
    size_t pos = file->Position(line, col);
    for (auto& msg : log->Messages()) {
        bool pos_ok = false;
        for (auto& loc : msg->Locators())
            if (loc.Position() == pos) {
                pos_ok = true;
                break;
            }
        if (!pos_ok)
            for (auto& loc : msg->SpanLocators())
                if (loc.Start().Position() <= pos && loc.End().Position() >= pos) {
                    pos_ok = true;
                    break;
                }
        if (!pos_ok) continue;
        stringstream ss;
        ss << msg->Code();
        msg->WriteMessageToStream(ss, complog::CompilationMessage::FormatOptions::All(80));
        string msgtext;
        for (char ch : ss.str())
            if (!isspace(ch)) msgtext.push_back(ch);
        if (msgtext.find(subMsg) != string::npos) return;
    }
    cout << "The test was expected to cause a message.\nLooked at 1-based-line " << line + 1 << ", col " << col
         << ", expected to find \"" << subMsg << "\"\nFull semantic analysis log:\n";
    log->WriteToStream(cout, complog::CompilationMessage::FormatOptions::All(80));
    ASSERT_FALSE(true) << "The compilation was expected to emit a message at a specific position";
}
