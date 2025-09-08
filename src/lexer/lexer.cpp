#include "lexer.h"
#include <stdexcept>

using namespace std;

static bool checkComments(size_t& i, size_t n, const string& code) {
    if (i + 1 < n && code[i] == '/' && code[i + 1] == '/') {
        while (i < n && code[i] != '\n') {
            i++;
        };
        return true;
    }
    return false;
}


static bool checkStringLiterals(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (code[i] == '"') {
        size_t position = i;
        string value;
        i++;
        while (i < n && code[i] != '"') {
            if (code [i] == '\n') {
                throw runtime_error("Newline character is not allowed in string literal");
            }
            value += code[i];
            i++;
        }
        i++;
        auto token = make_shared<StringLiteral>();
        token->type = Token::Type::tkStringLiteral;
        token->span = {position, i - position};
        token->value = value;
        tokens.push_back(token);
        return true;
    }
    return false;
}


static bool checkNumbers(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (isdigit(code[i])) {
        size_t position = i;
        long value = 0;
        while (i < n && isdigit(code[i])) {
            value = value * 10 + (code[i] - '0');
            i++;
        }
        if (i < n && code[i] == '.') {
            double realValue = value;
            double fraction = 0.1;
            i++;
            while (i < n && isdigit(code[i])) {
                realValue += (code[i] - '0') * fraction;
                fraction *= 0.1;
                i++;
            }
            auto token = make_shared<Real>();
            token->type = Token::Type::tkRealLiteral;
            token->span = {position, i - position};
            token->value = realValue;
            tokens.push_back(token);
        } else {
            auto token = make_shared<Integer>();
            token->type = Token::Type::tkIntLiteral;
            token->span = {position, i - position};
            token->value = value;
            tokens.push_back(token);
        }
        return true;
    }
    return false;
}


static bool checkToken(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    for (const pair<string, Token::Type>& tok : Token::typeChars) {
        if (i + tok.first.length() <= n && equal(tok.first.begin(), tok.first.end(), code.begin() + i)) {
            auto token = make_shared<Token>();
            token->type = tok.second;
            token->span = {i, tok.first.length()};
            tokens.push_back(token);
            i += tok.first.length();
            return true;
        }
    }
    return false;
}


static bool checkIdentifier(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (isalpha(code[i]) || code[i] == '_') {
        size_t position = i;
        string value;
        while (i < n && (isalnum(code[i]) || code[i] == '_')) {
            value += code[i];
            i++;
        }
        auto token = make_shared<Identifier>();
        token->type = Token::Type::tkIdent;
        token->span = {position, i - position};
        token->identifier = value;
        tokens.push_back(token);
        return true;
    }
    return false;
}

vector<shared_ptr<Token>> Lexer::tokenize(const string& code) {
    vector<shared_ptr<Token>> tokens;
    size_t i = 0;
    size_t n = code.length();
    while (i < code.length()) {
        if (code[i] == ' ' || code[i] == '\r' || code[i] == '\t') { i++; continue; }
        if (checkComments(i, n, code)) continue;
        if (checkStringLiterals(i, n, code, tokens)) continue;
        if (checkNumbers(i, n, code, tokens)) continue;
        if (checkToken(i, n, code, tokens)) continue;
        if (checkIdentifier(i, n, code, tokens)) continue;
        throw runtime_error(string("Unknown character: ") + code[i]);
    }
    return tokens;
}
