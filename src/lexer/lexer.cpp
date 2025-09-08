#include "lexer.h"
#include <stdexcept>

using namespace std;

static bool checkComments(size_t& i, size_t n, const string& code) {
    if (code.substr(i, 2) == "//") {
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
        std::string value;
        i++;
        while (i < n && code[i] != '"') {
            if (code [i] == '\n') {
                throw std::runtime_error("Newline character is not allowed in string literal");
            }
            value += code[i];
            i++;
        }
        i++;
        auto token = make_shared<StringLiteral>();
        token->type = Token::Type::tkStringLiteral;
        token->span = {position, value.length()};
        token->value = value;
        tokens.push_back(token);
        return true;
    }
    return false;
}


static bool checkNumbers(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (std::isdigit(code[i])) {
        size_t position = i;
        long value = 0;
        while (i < n && std::isdigit(code[i])) {
            value = value * 10 + (code[i] - '0');
            i++;
        }
        if (i < n && code[i] == '.') {
            double realValue = value;
            double fraction = 0.1;
            i++;
            while (i < n && std::isdigit(code[i])) {
                realValue += (code[i] - '0') * fraction;
                fraction *= 0.1;
                i++;
            }
            auto token = make_shared<Real>();
            token->type = Token::Type::tkRealLiteral;
            token->span = {position, i - position};
            token->value = value;
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
    for (const std::pair<std::string, Token::Type>& tok : Token::typeChars) {
        if (code.substr(i, tok.first.length()) == tok.first) {
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
    if (std::isalpha(code[i])) {
        size_t position = i;
        std::string value;
        while (i < n && (std::isalnum(code[i]) || code[i] == '_')) {
            value += code[i];
            i++;
        }
        auto token = make_shared<Identifier>();
        token->type = Token::Type::tkIdent;
        token->span = {position, value.length()};
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
        if (checkComments(i, n, code)) continue;
        if (checkStringLiterals(i, n, code, tokens)) continue;
        if (checkNumbers(i, n, code, tokens)) continue;
        if (checkToken(i, n, code, tokens)) continue;
        if (checkIdentifier(i, n, code, tokens)) continue;
    }
    return tokens;
}
