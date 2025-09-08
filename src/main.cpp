struct Span {
public:
    int line;
    int position;
};

class Token {
    Span span;
    unsigned int code;
};

class Lexer {
public:
    Token getNextToken() {
        
    }
};

class Syntax {
    
};

int main() {
    return 0;
}