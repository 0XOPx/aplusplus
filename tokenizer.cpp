// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#include "tokenizer.hpp"
#include <stdexcept>

const std::vector<std::string> Tokenizer::KEYWORDS = {
    "let", "var", "const", "func", "return", "if", "else", "elif",
    "while", "for", "in", "break", "continue", "class", "struct",
    "new", "delete", "null", "true", "false", "import", "export",
    "public", "private", "protected", "static", "override", "virtual",
    "int", "float", "double", "string", "bool", "char", "void",
    "list", "map", "set", "tuple", "enum", "match", "case", "default",
    "try", "catch", "throw", "finally", "async", "await", "yield",
    "print", "println", "input", "typeof", "sizeof", "as", "is",
    "and", "or", "not", "namespace", "use"
};

Tokenizer::Tokenizer(const std::string& filename)
    : filename(filename), line(1), column(1), pos(0) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    code = buffer.str();
}

Tokenizer::Tokenizer(const std::string& code, const std::string& filename)
    : filename(filename), code(code), line(1), column(1), pos(0) {}

std::vector<Token> Tokenizer::tokenize() {
    tokens.clear();
    pos = 0;
    line = 1;
    column = 1;

    while (!isAtEnd()) {
        char c = peek();

        if (c == '/' && peek(1) == '/') {
            tokenizeComment();
        } else if (c == '/' && peek(1) == '*') {
            tokenizeComment();
        } else if (c == '\n') {
            consume();
            line++;
            column = 1;
        } else if (isWhitespace(c)) {
            consume();
            column++;
        } else if (c == '"' || c == '\'') {
            if (c == '\'') tokenizeChar();
            else tokenizeString();
        } else if (isDigit(c)) {
            tokenizeNumber();
        } else if (isIdentifierStart(c)) {
            tokenizeIdentifier();
        } else if (isOperator(c)) {
            tokenizeOperator();
        } else if (isPunctuation(c)) {
            tokenizePunctuation();
        } else {
            error(std::string("Unexpected character: '") + c + "'");
            consume();
        }
    }

    addEndOfFile();
    return tokens;
}

std::vector<Token> Tokenizer::tokenize(const std::string& src) {
    code = src;
    return tokenize();
}

void Tokenizer::tokenizeIdentifier() {
    int startLine = line, startCol = column;
    std::string value;
    while (!isAtEnd() && isIdentifierPart(peek())) {
        value += consume();
        column++;
    }
    TokenType type = isKeyword(value) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
    if (value == "true" || value == "false") type = TokenType::BOOLEAN;
    tokens.emplace_back(type, value, startLine, startCol, filename);
}

void Tokenizer::tokenizeNumber() {
    int startLine = line, startCol = column;
    std::string value;
    bool isFloat = false;
    while (!isAtEnd() && (isDigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (isFloat) break;
            isFloat = true;
        }
        value += consume();
        column++;
    }
    tokens.emplace_back(isFloat ? TokenType::FLOAT : TokenType::INTEGER, value, startLine, startCol, filename);
}

void Tokenizer::tokenizeString() {
    int startLine = line, startCol = column;
    consume(); // opening "
    column++;
    std::string value;
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\\') {
            consume(); column++;
            char esc = consume(); column++;
            switch (esc) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default: value += esc; break;
            }
        } else {
            if (peek() == '\n') { line++; column = 1; }
            value += consume();
            column++;
        }
    }
    if (!isAtEnd()) { consume(); column++; } // closing "
    tokens.emplace_back(TokenType::STRING, value, startLine, startCol, filename);
}

void Tokenizer::tokenizeChar() {
    int startLine = line, startCol = column;
    consume(); // opening '
    column++;
    std::string value;
    if (!isAtEnd() && peek() == '\\') {
        consume(); column++;
        value += consume(); column++;
    } else if (!isAtEnd()) {
        value += consume(); column++;
    }
    if (!isAtEnd() && peek() == '\'') { consume(); column++; }
    tokens.emplace_back(TokenType::CHAR, value, startLine, startCol, filename);
}

void Tokenizer::tokenizeComment() {
    if (peek(1) == '/') {
        while (!isAtEnd() && peek() != '\n') consume();
    } else {
        consume(); consume(); // /*
        while (!isAtEnd()) {
            if (peek() == '*' && peek(1) == '/') {
                consume(); consume();
                break;
            }
            if (peek() == '\n') { line++; column = 1; }
            consume();
        }
    }
}

void Tokenizer::tokenizeOperator() {
    int startLine = line, startCol = column;
    std::string value;
    value += consume();
    column++;

    // Two-character operators
    if (!isAtEnd()) {
        char next = peek();
        std::string two = value + next;
        if (two == "==" || two == "!=" || two == "<=" || two == ">=" ||
            two == "+=" || two == "-=" || two == "*=" || two == "/=" ||
            two == "++" || two == "--" || two == "->" || two == "=>" ||
            two == "&&" || two == "||" || two == "::" || two == ".." ||
            two == "<<" || two == ">>") {
            value += consume();
            column++;
        }
    }
    tokens.emplace_back(TokenType::OPERATOR, value, startLine, startCol, filename);
}

void Tokenizer::tokenizePunctuation() {
    int startLine = line, startCol = column;
    std::string value(1, consume());
    column++;
    tokens.emplace_back(TokenType::PUNCTUATION, value, startLine, startCol, filename);
}

void Tokenizer::addEndOfFile() {
    tokens.emplace_back(TokenType::END_OF_FILE, "", line, column, filename);
}

char Tokenizer::peek(int offset) const {
    size_t idx = pos + offset;
    if (idx >= code.size()) return '\0';
    return code[idx];
}

char Tokenizer::consume() {
    if (isAtEnd()) return '\0';
    return code[pos++];
}

bool Tokenizer::isAtEnd() const { return pos >= code.size(); }
bool Tokenizer::isIdentifierStart(char c) const { return std::isalpha(c) || c == '_'; }
bool Tokenizer::isIdentifierPart(char c) const { return std::isalnum(c) || c == '_'; }
bool Tokenizer::isDigit(char c) const { return std::isdigit(c); }
bool Tokenizer::isWhitespace(char c) const { return c == ' ' || c == '\t' || c == '\r'; }
bool Tokenizer::isPunctuation(char c) const {
    static const std::string puncts = "(){}[];,.:@#";
    return puncts.find(c) != std::string::npos;
}
bool Tokenizer::isOperator(char c) const {
    static const std::string ops = "+-*/=<>!&|^~%?";
    return ops.find(c) != std::string::npos;
}
bool Tokenizer::isKeyword(const std::string& s) const {
    return std::find(KEYWORDS.begin(), KEYWORDS.end(), s) != KEYWORDS.end();
}
void Tokenizer::error(const std::string& message) const {
    throw std::runtime_error("[Tokenizer] " + filename + ":" +
                             std::to_string(line) + ":" +
                             std::to_string(column) + ": " + message);
}
