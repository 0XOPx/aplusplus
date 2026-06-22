// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include "tokenizer.hpp"
#include "ast.hpp"
#include "error_handler.hpp"

class Parser {
public:
    Parser(const std::vector<Token>& tokens, ErrorHandler& err);

    ASTNodePtr parse();

private:
    std::vector<Token> tokens_;
    size_t             pos_;
    ErrorHandler&      err_;

    Token peek(int offset = 0) const;
    Token consume();
    Token expect(TokenType type, const std::string& value = "");
    bool  check(TokenType type, const std::string& value = "") const;
    bool  match(TokenType type, const std::string& value = "");
    bool  isAtEnd() const;

    ASTNodePtr parseProgram();
    ASTNodePtr parseStatement();
    ASTNodePtr parseBlock();
    ASTNodePtr parseVarDecl(bool isConst = false);
    ASTNodePtr parseFuncDecl();
    ASTNodePtr parseClassDecl();
    ASTNodePtr parseReturnStmt();
    ASTNodePtr parseIfStmt();
    ASTNodePtr parseWhileStmt();
    ASTNodePtr parseForStmt();
    ASTNodePtr parseBreakStmt();
    ASTNodePtr parseContinueStmt();
    ASTNodePtr parsePrintStmt();
    ASTNodePtr parseImportStmt();
    ASTNodePtr parseMatchStmt();
    ASTNodePtr parseTryStmt();
    ASTNodePtr parseExprStmt();

    ASTNodePtr parseExpression();
    ASTNodePtr parseAssignment();
    ASTNodePtr parseTernary();
    ASTNodePtr parseOr();
    ASTNodePtr parseAnd();
    ASTNodePtr parseEquality();
    ASTNodePtr parseComparison();
    ASTNodePtr parseTerm();
    ASTNodePtr parseFactor();
    ASTNodePtr parseUnary();
    ASTNodePtr parsePostfix();
    ASTNodePtr parsePrimary();
    ASTNodePtr parseCall(ASTNodePtr callee);
    ASTNodePtr parseListLiteral();
    ASTNodePtr parseMapLiteral();
    ASTNodePtr parseLambda();

    std::string parseTypeName();
    std::vector<ASTNodePtr> parseParams();
    std::vector<ASTNodePtr> parseArgs();

    void error(const std::string& msg, int line = 0, int col = 0);
    void synchronize();
};

#endif // PARSER_HPP
