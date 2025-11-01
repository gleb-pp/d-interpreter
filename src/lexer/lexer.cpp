#include "lexer.h"

#include <algorithm>
#include <cctype>
#include <utility>

#include "complog/CompilationLog.h"
#include "complog/CompilationMessage.h"
#include "locators/CodeFile.h"
#include "locators/locator.h"

using namespace std;

static bool isLatin(char ch) { return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'); }

LexerError::LexerError(const locators::Locator& position)
    : complog::CompilationMessage(complog::Severity::Error(), "LexerError"), position(position) {}

void LexerError::WriteMessageToStream(std::ostream& out, [[maybe_unused]] const FormatOptions& opts) const {
    out << "Cannot tokenize the file: error at " << position.Pretty() << ".\n";
}

std::vector<locators::Locator> LexerError::Locators() const { return {position}; }

NewlineInStringLiteralError::NewlineInStringLiteralError(const locators::Locator& position)
    : complog::CompilationMessage(complog::Severity::Error(), "EolnInStringError"), position(position) {}

void NewlineInStringLiteralError::WriteMessageToStream(std::ostream& out,
                                                       [[maybe_unused]] const FormatOptions& opts) const {
    out << "A string literal cannot span several lines.\n" << "Line break at " << position.Pretty() << ".\n";
}

std::vector<locators::Locator> NewlineInStringLiteralError::Locators() const { return {position}; }

WrongEscapeSequenceError::WrongEscapeSequenceError(const locators::Locator& position, const std::string& badsequence)
    : complog::CompilationMessage(complog::Severity::Error(), "EscapeSequenceError"),
      position(position),
      badsequence(badsequence) {}

void WrongEscapeSequenceError::WriteMessageToStream(std::ostream& out,
                                                    [[maybe_unused]] const FormatOptions& opts) const {
    out << "At " << position.Pretty() << ": this escape sequence is not supported: \"" << badsequence << "\".\n";
}

std::vector<locators::Locator> WrongEscapeSequenceError::Locators() const { return {position}; }

UnclosedStringLiteralError::UnclosedStringLiteralError(const locators::Locator& position)
    : complog::CompilationMessage(complog::Severity::Error(), "UnclosedStringLiteralError"), position(position) {}

void UnclosedStringLiteralError::WriteMessageToStream(std::ostream& out,
                                                      [[maybe_unused]] const FormatOptions& opts) const {
    out << "Closing quote expected at " << position.Pretty() << ".\n";
}

std::vector<locators::Locator> UnclosedStringLiteralError::Locators() const { return {position}; }

const std::vector<std::pair<std::string, Token::Type>> Token::typeChars = {
    std::make_pair("var", Token::Type::tkVar),
    std::make_pair("while", Token::Type::tkWhile),
    std::make_pair("for", Token::Type::tkFor),
    std::make_pair("if", Token::Type::tkIf),
    std::make_pair("then", Token::Type::tkThen),
    std::make_pair("end", Token::Type::tkEnd),
    std::make_pair("exit", Token::Type::tkExit),
    std::make_pair("print", Token::Type::tkPrint),
    std::make_pair("else", Token::Type::tkElse),
    std::make_pair("loop", Token::Type::tkLoop),
    std::make_pair(",", Token::Type::tkComma),
    std::make_pair("and", Token::Type::tkAnd),
    std::make_pair("or", Token::Type::tkOr),
    std::make_pair("not", Token::Type::tkNot),
    std::make_pair("xor", Token::Type::tkXor),
    std::make_pair("real", Token::Type::tkReal),
    std::make_pair("string", Token::Type::tkString),
    std::make_pair("bool", Token::Type::tkBool),
    std::make_pair("none", Token::Type::tkNone),
    std::make_pair("func", Token::Type::tkFunc),
    std::make_pair("+", Token::Type::tkPlus),
    std::make_pair("-", Token::Type::tkMinus),
    std::make_pair("*", Token::Type::tkTimes),
    std::make_pair("\n", Token::Type::tkNewLine),
    std::make_pair("[", Token::Type::tkOpenBracket),
    std::make_pair("]", Token::Type::tkClosedBracket),
    std::make_pair("(", Token::Type::tkOpenParenthesis),
    std::make_pair(")", Token::Type::tkClosedParenthesis),
    std::make_pair("{", Token::Type::tkOpenCurlyBrace),
    std::make_pair("}", Token::Type::tkClosedCurlyBrace),
    std::make_pair(";", Token::Type::tkSemicolon),
    std::make_pair(":=", Token::Type::tkAssign),
    std::make_pair("true", Token::Type::tkTrue),
    std::make_pair("false", Token::Type::tkFalse),
    std::make_pair("is", Token::Type::tkIs),
    std::make_pair("return", Token::Type::tkReturn),

    std::make_pair("int", Token::Type::tkInt),
    std::make_pair("in", Token::Type::tkIn),

    std::make_pair("..", Token::Type::tkRange),
    std::make_pair(".", Token::Type::tkDot),

    std::make_pair("=>", Token::Type::tkArrow),
    std::make_pair("=", Token::Type::tkEqual),

    std::make_pair(">=", Token::Type::tkGreaterEq),
    std::make_pair(">", Token::Type::tkGreater),

    std::make_pair("<=", Token::Type::tkLessEq),
    std::make_pair("<", Token::Type::tkLess),

    std::make_pair("/=", Token::Type::tkNotEqual),
    std::make_pair("/", Token::Type::tkDivide)};

string Token::TypeToString(Token::Type type) {
    switch (type) {
        case Token::Type::tkIntLiteral:
            return "<int literal>";
        case Token::Type::tkRealLiteral:
            return "<real literal>";
        case Token::Type::tkStringLiteral:
            return "<string literal>";
        case Token::Type::tkIdent:
            return "<identifier>";
        case Token::Type::tkEof:
            return "<end of file>";
        case Token::Type::tkNewLine:
            return "<line break>";
        default:
            break;
    }
    for (auto& p : Token::typeChars) {
        if (p.second == type) return "\"" + p.first + "\"";
    }
    return "<?>";
}

static bool checkComments(size_t& i, size_t n, const string& code) {
    if (i + 1 < n && code[i] == '/' && code[i + 1] == '/') {
        while (i < n && code[i] != '\n') {
            i++;
        };
        return true;
    }
    return false;
}

static bool checkStringLiterals(size_t& i, size_t n, const shared_ptr<const locators::CodeFile>& file,
                                vector<shared_ptr<Token>>& tokens, complog::ICompilationLog& log) {
    const std::string& code = file->AllText();
    if (code[i] != '"') return false;
    size_t position = i;
    string value;
    i++;
    while (i < n && code[i] != '"') {
        if (code[i] == '\n') {
            log.Log(make_shared<NewlineInStringLiteralError>(locators::Locator(file, i)));
            i = position;
            return false;
        } else if (code[i] == '\\' && i + 1 < n && code[i + 1] == 'n') {
            value += '\n';
            i += 2;
        } else if (code[i] == '\\' && i + 1 < n && code[i + 1] == 't') {
            value += '\t';
            i += 2;
        } else if (code[i] == '\\' && i + 1 < n && code[i + 1] == 'r') {
            value += '\r';
            i += 2;
        } else if (code[i] == '\\' && i + 1 < n && code[i + 1] == '"') {
            value += '"';
            i += 2;
        } else if (code[i] == '\\' && i + 1 < n && code[i + 1] == '\\') {
            value += '\\';
            i += 2;
        } else {
            value += code[i];
            i++;
        }
    }
    if (i == n) {
        log.Log(make_shared<UnclosedStringLiteralError>(locators::Locator(file, i)));
        i = position;
        return false;
    }
    i++;
    auto token = make_shared<StringLiteral>();
    token->type = Token::Type::tkStringLiteral;
    token->span = {position, i - position};
    token->value = value;
    tokens.push_back(token);
    return true;
}

static bool checkNumbers(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (!isdigit(code[i])) return false;
    size_t position = i;
    BigInt value = 0;
    while (i < n && isdigit(code[i])) {
        value = value * BigInt(10) + BigInt(code[i] - '0');
        i++;
    }
    if (i < n && code[i] == '.' && i + 1 < n && isdigit(code[i + 1])) {
        long double realValue = value.ToFloat();
        long double fraction = 0.1;
        i++;
        while (i < n && isdigit(code[i])) {
            realValue += (code[i] - '0') * fraction;
            fraction *= 0.1;
            i++;
        }
        auto token = make_shared<RealToken>();
        token->type = Token::Type::tkRealLiteral;
        token->span = {position, i - position};
        token->value = realValue;
        tokens.push_back(token);
    } else {
        auto token = make_shared<IntegerToken>();
        token->type = Token::Type::tkIntLiteral;
        token->span = {position, i - position};
        token->value = value;
        tokens.push_back(token);
    }
    return true;
}

static Token::Type alphabeticTokens[] = {
    /*19*/ Token::Type::tkVar,
    /*20*/ Token::Type::tkWhile,
    /*21*/ Token::Type::tkFor,
    /*22*/ Token::Type::tkIf,
    /*23*/ Token::Type::tkThen,
    /*24*/ Token::Type::tkEnd,
    /*26*/ Token::Type::tkExit,
    /*27*/ Token::Type::tkPrint,
    /*29*/ Token::Type::tkIn,
    /*30*/ Token::Type::tkElse,
    /*31*/ Token::Type::tkLoop,
    /*34*/ Token::Type::tkAnd,
    /*35*/ Token::Type::tkOr,
    /*36*/ Token::Type::tkNot,
    /*37*/ Token::Type::tkXor,
    /*38*/ Token::Type::tkInt,
    /*39*/ Token::Type::tkReal,
    /*40*/ Token::Type::tkBool,
    /*41*/ Token::Type::tkString,
    /*42*/ Token::Type::tkNone,
    /*43*/ Token::Type::tkFunc,
    /*44*/ Token::Type::tkTrue,
    /*45*/ Token::Type::tkFalse,
    /*46*/ Token::Type::tkIs,
    /*47*/ Token::Type::tkReturn,
};  // VERY IMPORTANT that this is sorted

static bool isAlphabeticToken(Token::Type tktype) {  // O(log sizeof(alphabeticTokens))
    constexpr size_t N = sizeof(alphabeticTokens) / sizeof(alphabeticTokens[0]);
    auto found = std::lower_bound(alphabeticTokens, alphabeticTokens + N, tktype) - alphabeticTokens;
    return found < static_cast<long>(N) && alphabeticTokens[found] == tktype;
}

static bool checkToken(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    for (const pair<string, Token::Type>& tok : Token::typeChars) {
        if (i + tok.first.length() <= n && equal(tok.first.begin(), tok.first.end(), code.begin() + i)) {
            auto tktype = tok.second;
            if (isAlphabeticToken(tktype) && i + tok.first.length() < n) {
                char nx = code[i + tok.first.length()];
                if (isLatin(nx) || isdigit(nx) || nx == '_') continue;
            }
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

static bool checkIdentifierToken(size_t& i, size_t n, const string& code, vector<shared_ptr<Token>>& tokens) {
    if (!isLatin(code[i]) && code[i] != '_') return false;
    size_t position = i;
    while (i < n && (isLatin(code[i]) || isdigit(code[i]) || code[i] == '_')) i++;
    string value = code.substr(position, i - position);
    auto token = make_shared<IdentifierToken>();
    token->type = Token::Type::tkIdent;
    token->span = {position, i - position};
    token->identifier = value;
    tokens.push_back(token);
    return true;
}

optional<vector<shared_ptr<Token>>> Lexer::tokenize(const shared_ptr<const locators::CodeFile>& file,
                                                    complog::ICompilationLog& log) {
    vector<shared_ptr<Token>> tokens;
    size_t i = 0;
    const std::string& code = file->AllText();
    size_t n = code.length();
    while (i < n) {
        if (code[i] == ' ' || code[i] == '\r' || code[i] == '\t') {
            i++;
            continue;
        }
        if (checkComments(i, n, code)) continue;
        if (checkStringLiterals(i, n, file, tokens, log)) continue;
        if (checkNumbers(i, n, code, tokens)) continue;
        if (checkToken(i, n, code, tokens)) continue;
        if (checkIdentifierToken(i, n, code, tokens)) continue;
        log.Log(make_shared<LexerError>(locators::Locator(file, i)));
        return {};
    }
    auto eof = make_shared<Token>();
    eof->type = Token::Type::tkEof;
    eof->span = {n, 0};
    tokens.push_back(eof);
    return tokens;
}
