#include "syntax.h"
#include "complog/CompilationMessage.h"
#include "locators/locator.h"
using namespace std;

class UnexpectedTokenTypeError : public complog::CompilationMessage {
private:
    locators::Locator loc;
    Token::Type expected, found;

public:
    UnexpectedTokenTypeError(locators::Locator position, Token::Type expected, Token::Type found)
    : CompilationMessage(complog::Severity::Error(), "UnexpectedTokenTypeError"),
        loc(position), expected(expected), found(found) {}
    void WriteMessageToStream(std::ostream& out, const FormatOptions& options) const override {
        out << "Unexpected token at " << loc.Pretty() << "; expected " <<
            Token::TypeToString(expected) << ", but found " << Token::TypeToString(found) << ".\n";
    }
    std::vector<locators::Locator> Locators() const override {
        return { loc };
    }
    virtual ~UnexpectedTokenTypeError() override = default;
};

bool AssertToken(SyntaxContext& context, size_t pos, Token::Type expected) {
    auto& token = *context.tokens[pos];
    if (token.type == expected)
        return true;
    size_t textpos = token.span.position;
    context.report.Report(textpos, make_shared<UnexpectedTokenTypeError>(
                          context.MakeLocator(textpos), expected, token.type));
    return false;
}

locators::Locator SyntaxContext::MakeLocator(size_t pos) const {
    return locators::Locator(file, pos);
}

namespace ast {

bool parseSep(const vector<shared_ptr<Token>>& tokens, size_t& pos) {
    size_t n = tokens.size();
    if (pos >= n) return false;
}

}
