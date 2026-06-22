// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>

enum class TokenType {
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    KEYWORD,
    OPERATOR,
    PUNCTUATION,
    COMMENT,
    WHITESPACE,
    END_OF_FILE,
    BOOLEAN,
    NEWLINE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    std::string filename;

    Token(TokenType t, const std::string& v, int l, int c, const std::string& f = "")
        : type(t), value(v), line(l), column(c), filename(f) {}
};

class Tokenizer {
public:
    Tokenizer(const std::string& filename);
    Tokenizer(const std::string& code, const std::string& filename);
    std::vector<Token> tokenize();
    std::vector<Token> tokenize(const std::string& code);

private:
    std::string filename;
    std::string code;
    int line;
    int column;
    size_t pos;
    std::vector<Token> tokens;

    static const std::vector<std::string> KEYWORDS;

    void tokenizeIdentifier();
    void tokenizeNumber();
    void tokenizeString();
    void tokenizeChar();
    void tokenizeComment();
    void tokenizeWhitespace();
    void tokenizeOperator();
    void tokenizePunctuation();
    void addEndOfFile();

    char peek(int offset = 0) const;
    char consume();
    void skipWhitespace();

    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool isDigit(char c) const;
    bool isOperator(char c) const;
    bool isPunctuation(char c) const;
    bool isWhitespace(char c) const;
    bool isKeyword(const std::string& s) const;
    bool isAtEnd() const;

    void error(const std::string& message) const;
};

#endif // TOKENIZER_HPP
