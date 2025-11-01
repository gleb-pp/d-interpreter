#include "syntax.h"
using namespace std;

TokenScanner::AutoBlock::AutoBlock(TokenScanner* owner) : tk(owner) {}
void TokenScanner::AutoBlock::Success() { success = true; }
TokenScanner::AutoBlock::~AutoBlock() { if (success) tk->EndSuccess(); else tk->EndFail(); }

TokenScanner::StackBlock::StackBlock(int index, bool ignoreEoln)
    : StartIndex(index), Index(index), IgnoreEoln(ignoreEoln) {}

size_t TokenScanner::StartOfToken(size_t index) const {
    if (tokens.empty()) return 0;
    if (index >= tokens.size()) return EndOfToken(tokens.size() - 1);
    return tokens[index]->span.position;
}

size_t TokenScanner::EndOfToken(size_t index) const {
    if (tokens.empty()) return 0;
    auto& span = tokens[min(tokens.size() - 1, index)]->span;
    return span.position + span.length;
}

void TokenScanner::SkipEolns() {
    while (Read(Token::Type::tkNewLine)) {}
}

TokenScanner::TokenScanner(const std::vector<std::shared_ptr<Token>>& tokens, const std::shared_ptr<const locators::CodeFile>& file)
    : codeFile(file), tokens(tokens), stack({{0, false}}), report(file) {}

locators::Locator TokenScanner::PositionInFile() const {
    return locators::Locator(codeFile, StartOfToken(Index()));
}

locators::Locator TokenScanner::StartPositionInFile() const {
    return locators::Locator(codeFile, StartOfToken(stack.back().StartIndex));
}

locators::SpanLocator TokenScanner::ReadSinceStart() const {
    auto& topframe = stack.back();
    int startind = topframe.StartIndex;
    if (startind == topframe.Index) return locators::SpanLocator(codeFile, StartOfToken(startind), 0);
    int endind = topframe.Index - 1;
    if (topframe.IgnoreEoln) {
        while (endind >= startind && tokens[endind]->type == Token::Type::tkNewLine) --endind;
        if (endind < startind) return locators::SpanLocator(codeFile, StartOfToken(startind), 0);
        while (tokens[startind]->type == Token::Type::tkNewLine) ++startind;
    }
    size_t start = StartOfToken(startind);
    size_t end = EndOfToken(endind);
    return locators::SpanLocator(codeFile, start, end - start);
}

size_t TokenScanner::Index() const { return stack.back().Index; }

const std::vector<std::shared_ptr<Token>>& TokenScanner::Tokens() const { return tokens; }

void TokenScanner::Start() {
    auto& topframe = stack.back();
    stack.emplace_back(topframe.Index, topframe.IgnoreEoln);
}

void TokenScanner::StartIgnoreEoln() {
    stack.emplace_back(Index(), true);
    SkipEolns();
}

void TokenScanner::StartUseEoln() {
    stack.emplace_back(Index(), false);
}

void TokenScanner::EndFail() {
    stack.pop_back();
}

void TokenScanner::EndSuccess() {
    auto topframe = stack.back();
    stack.pop_back();
    auto& prevframe = stack.back();
    prevframe.Index = topframe.Index;
    if (prevframe.IgnoreEoln) SkipEolns();
}

std::shared_ptr<Token> TokenScanner::Peek() {
    return tokens[Index()];
}

std::shared_ptr<Token> TokenScanner::Read() {
    auto& res = tokens[Index()];
    Advance();
    return res;
}

void TokenScanner::Advance(size_t count) {
    int& index = stack.back().Index;
    index = min(static_cast<int>(tokens.size()) - 1, static_cast<int>(index + count));
}

std::optional<std::shared_ptr<Token>> TokenScanner::Read(Token::Type type) {
    auto& res = tokens[Index()];
    if (res->type != type) {
        report.ReportUnexpectedToken(res->span.position, type, res->type);
        return {};
    }
    Advance();
    if (stack.back().IgnoreEoln) SkipEolns();
    return res;
}

SyntaxErrorReport& TokenScanner::Report() {
    return report;
}

const SyntaxErrorReport& TokenScanner::Report() const {
    return report;
}

TokenScanner::AutoBlock TokenScanner::AutoStart() {
    Start();
    return AutoBlock(this);
}

TokenScanner::AutoBlock TokenScanner::AutoStartIgnoreEoln() {
    StartIgnoreEoln();
    return AutoBlock(this);
}

TokenScanner::AutoBlock TokenScanner::AutoStartUseEoln() {
    StartUseEoln();
    return AutoBlock(this);
}
