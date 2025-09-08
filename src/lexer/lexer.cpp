#include "lexer.h"
#include <csignal>
#include <optional>
using namespace std;

static inline bool isWhitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
}

static size_t skipWhitespaceAndComments(const string& code, size_t pos) {
    size_t n = code.size();
    while (pos < n && isWhitespace(code[pos])) ++pos;
    if (pos + 2 <= n && code[pos] == '/' && code[pos + 1] == '/') {
        pos += 2;
        while (pos < n && code[pos] != '\n') ++pos;
    }
    return pos;
}

static bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

static optional<shared_ptr<IntegerToken>> TryScanInt(const string& code, size_t pos) {
    size_t n = code.size();
    size_t start = pos;
    long value = 0;
    while (pos < n && isDigit(code[pos]))
        value = 10 * value + (code[pos++] - '0');
    if (start == pos)
        return {};
    Span span = {start, pos - start};
    return make_shared<IntegerToken>(span, value);
}

static optional<shared_ptr<RealToken>> TryScanReal(const string& code, size_t pos) {
    size_t n = code.size();
    size_t start = pos;
    long double value = 0;
    while (pos < n && isDigit(code[pos])) {
        value = 10 * value + (code[pos++] - '0');
    }
    if (start == pos) return {};
    if (pos >= n || code[pos] != '.') return {};
    ++pos;
    long double multiplier = 0.1;
    while (pos < n && isDigit(code[pos])) {
        value += multiplier * (code[pos] - '0');
        multiplier *= 0.1;
    }
    Span span = { start, pos - start };
    return make_shared<RealToken>(span, value);
}

static bool isHexDigit(char ch) {
    return isDigit(ch) || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}

static int getHexValue(char ch) {
    if (isDigit(ch))
        return ch - '0';
    if ('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;
    if ('a' <= ch && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

static optional<pair<size_t, char>> tryScanOneCharInStringLiteral(const string& code, size_t pos) {
    size_t n = code.size();
    char ch = code[pos++];
    if (ch == '\n')
        return {};
    if (ch == '\\') {
        if (pos >= n) return {};
        char escaped = code[pos++];
        if (escaped == 'n')
            return make_pair(2, '\n');
        if (escaped == 't')
            return make_pair(2, '\t');
        if (escaped == 'r')
            return make_pair(2, '\r');
        if (escaped == '"')
            return make_pair(2, '"');
        if (escaped == '\\')
            return make_pair(2, '\\');
        if (escaped == 'x') {
            if (pos + 2 > n) return {};
            int value = 0;
            for (int i = 0; i < 2; i++) {
                int dig = getHexValue(code[pos + i]);
                if (dig == -1)
                    return {};
                value = value * 16 + dig;
            }
            return make_pair(4, static_cast<char>(value));
        }
        return {};
    }
    return make_pair(1, ch);
}

static optional<shared_ptr<StringLiteral>> TryScanStringLiteral(const string& code, size_t pos) {
    size_t n = code.size(), start = pos;
    string value;
    if (pos >= n || code[pos] != '"') return {};
    ++pos;
    while (true) {
        if (pos >= n)
            return {};
        if (code[pos] == '"')
            break;
        auto maybeChar = tryScanOneCharInStringLiteral(code, pos);
        if (!maybeChar.has_value()) return {};
        pos += maybeChar->first;
        value.push_back(maybeChar->second);
    }
    Span span = {start, pos - start};
    return make_shared<StringLiteral>(span, value);
}

static bool isLatinOr_(char ch) {
    return ('a' <= ch && ch <= 'z') ||
        ('A' <= ch && ch <= 'Z') ||
        ch == '_';
}

static optional<shared_ptr<IdentifierToken>> TryScanIdent(const string& code, size_t pos) {
    size_t n = code.size(), start = pos;
    if (pos >= n || !isLatinOr_(code[pos]))
        return {};
    ++pos;
    while (pos < n && (isLatinOr_(code[pos]) || isDigit(code[pos]))) {
        ++pos;
    }
    Span span = { start, pos - start };
    return make_shared<IdentifierToken>(span, code.substr(start, pos - start));
}

static optional<shared_ptr<Token>> TryScanToken(const string& code, size_t pos) {
    size_t n = code.size();
    size_t remainingChars = n - pos;
    size_t longest = 0;
    size_t foundTokenType = 0;
    bool found = false;
    constexpr size_t controlCount = sizeof(Token::typeStrs) / sizeof(Token::typeStrs[0]);
    for (size_t i = 0; i < controlCount; i++) {
        string typestr = Token::typeStrs[i];
        if (typestr.size() > remainingChars || typestr.size() < longest) continue;
        if (code.substr(pos, typestr.size()) == typestr) {
            longest = typestr.size();
            foundTokenType = i;
            found = true;
        }
    }
    if (found) {
        Span span { pos, longest };
        return make_shared<Token>(span, static_cast<Token::Type>(foundTokenType));
    }

    auto maybeInt = TryScanInt(code, pos);
    if (maybeInt.has_value())
        return {maybeInt.value()};
    auto maybeReal = TryScanReal(code, pos);
    if (maybeReal.has_value())
        return {maybeReal.value()};
    auto maybeIdent = TryScanIdent(code, pos);
    if (maybeIdent.has_value())
        return {maybeIdent.value()};
    auto maybeStringLiteral = TryScanStringLiteral(code, pos);
    if (maybeStringLiteral.has_value())
        return {maybeStringLiteral.value()};
    return {};
}

vector<shared_ptr<Token>> Lexer::tokenize(const string& code) {
    vector<shared_ptr<Token>> tokens;
    size_t pos = skipWhitespaceAndComments(code, 0);
    size_t n = code.size();
    while (pos < n) {
        auto maybeToken = TryScanToken(code, pos);
        if (!maybeToken.has_value())
            throw pos;
        tokens.push_back(maybeToken.value());
        pos += maybeToken.value()->span.length;
        pos = skipWhitespaceAndComments(code, pos);
    }
    return tokens;
}

Token::Token(Span span, Token::Type tkType) : span(span), type(tkType) { }

IdentifierToken::IdentifierToken(Span span, const string& value) : Token(span, Token::Type::tkIdent), identifier(value) {}

IntegerToken::IntegerToken(Span span, long value) : Token(span, Token::Type::tkIntLiteral), value(value) {}

RealToken::RealToken(Span span, long double value) : Token(span, Token::Type::tkRealLiteral), value(value) {}

StringLiteral::StringLiteral(Span span, const string& value) : Token(span, Token::Type::tkRealLiteral), value(value) {}
