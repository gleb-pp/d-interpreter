#include "dinterp/asterrors.h"
#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/lexer.h"
#include "dinterp/syntax.h"
using namespace std;

EmptyVarStatement::EmptyVarStatement(locators::Locator position)
    : CompilationMessage(complog::Severity::Error(), "EmptyVarStatement"), loc(position) {}
void EmptyVarStatement::WriteMessageToStream(ostream& out, [[maybe_unused]] const FormatOptions& options) const {
    out << "The \"var\" statement at " << loc.Pretty() << " must contain at least one declaration.\n";
}
vector<locators::Locator> EmptyVarStatement::Locators() const { return {loc}; }

UnexpectedTokenTypeError::UnexpectedTokenTypeError(locators::Locator position, const vector<Token::Type>& expected,
                                                   Token::Type found)
    : CompilationMessage(complog::Severity::Error(), "UnexpectedTokenTypeError"),
      loc(position),
      expected(expected),
      found(found) {}
void UnexpectedTokenTypeError::WriteMessageToStream(ostream& out, [[maybe_unused]] const FormatOptions& options) const {
    out << "Unexpected token at " << loc.Pretty() << "; expected ";
    if (!expected.empty()) {
        out << Token::TypeToString(expected[0]);
        if (expected.size() == 2)
            out << " or " << Token::TypeToString(expected[1]);
        else {
            size_t n = expected.size();
            for (size_t i = 1; i < n; ++i) out << ", or " << Token::TypeToString(expected[i]);
        }
    }
    out << ", but found " << Token::TypeToString(found) << ".\n";
}
vector<locators::Locator> UnexpectedTokenTypeError::Locators() const { return {loc}; }

SyntaxErrorReport::SyntaxErrorReport(const shared_ptr<const locators::CodeFile>& file) : file(file) {}
void SyntaxErrorReport::ReportUnexpectedToken(size_t pos, Token::Type expected, Token::Type found) {
    if (rightmostPos > pos) return;
    if (rightmostPos < pos) {
        rightmostPos = pos;
        unexpTokens.clear();
    }
    unexpTokens[found].insert(expected);
}
vector<shared_ptr<complog::CompilationMessage>> SyntaxErrorReport::MakeReport() const {
    vector<shared_ptr<complog::CompilationMessage>> res;
    for (auto& p : unexpTokens) {
        res.push_back(make_shared<UnexpectedTokenTypeError>(
            locators::Locator(file, rightmostPos), vector<Token::Type>(p.second.begin(), p.second.end()), p.first));
    }
    return res;
}

SyntaxContext::SyntaxContext(const std::vector<std::shared_ptr<Token>>& tokens,
                             const std::shared_ptr<const locators::CodeFile>& file, complog::ICompilationLog& complog)
    : tokens(tokens, file), compilationLog(&complog) {}
