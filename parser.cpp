// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#include "parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens, ErrorHandler& err)
    : tokens_(tokens), pos_(0), err_(err) {}

ASTNodePtr Parser::parse() { return parseProgram(); }

// ── Helpers ──────────────────────────────────────────────────────────────────

Token Parser::peek(int offset) const {
    size_t idx = pos_ + offset;
    if (idx >= tokens_.size())
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    return tokens_[idx];
}

Token Parser::consume() {
    if (!isAtEnd()) return tokens_[pos_++];
    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

bool Parser::check(TokenType type, const std::string& value) const {
    if (isAtEnd()) return false;
    auto& t = tokens_[pos_];
    if (t.type != type) return false;
    if (!value.empty() && t.value != value) return false;
    return true;
}

bool Parser::match(TokenType type, const std::string& value) {
    if (check(type, value)) { consume(); return true; }
    return false;
}

Token Parser::expect(TokenType type, const std::string& value) {
    if (check(type, value)) return consume();
    auto& t = peek();
    error("Expected '" + (value.empty() ? "token" : value) + "' but got '" + t.value + "'", t.line, t.column);
    return consume();
}

bool Parser::isAtEnd() const {
    return pos_ >= tokens_.size() || tokens_[pos_].type == TokenType::END_OF_FILE;
}

void Parser::error(const std::string& msg, int line, int col) {
    err_.report(ErrorCode::SYNTAX_ERROR, msg, line, col);
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        auto t = peek();
        if (t.type == TokenType::KEYWORD &&
            (t.value == "func" || t.value == "let" || t.value == "var" ||
             t.value == "if" || t.value == "while" || t.value == "for" ||
             t.value == "return" || t.value == "class")) return;
        if (t.type == TokenType::PUNCTUATION && t.value == "}") return;
        consume();
    }
}

// ── Program / Block ───────────────────────────────────────────────────────────

ASTNodePtr Parser::parseProgram() {
    auto root = makeNode(NodeType::PROGRAM, "", 1, 1);
    while (!isAtEnd()) {
        try { root->addChild(parseStatement()); }
        catch (const AppError&) { synchronize(); }
        catch (const std::exception& e) {
            err_.report(ErrorCode::SYNTAX_ERROR, e.what());
            synchronize();
        }
    }
    return root;
}

ASTNodePtr Parser::parseBlock() {
    expect(TokenType::PUNCTUATION, "{");
    auto block = makeNode(NodeType::BLOCK, "", peek().line, peek().column);
    while (!isAtEnd() && !check(TokenType::PUNCTUATION, "}")) {
        try { block->addChild(parseStatement()); }
        catch (const AppError&) { synchronize(); }
    }
    expect(TokenType::PUNCTUATION, "}");
    return block;
}

// ── Statements ─────────────────────────────────────────────────────────────

ASTNodePtr Parser::parseStatement() {
    auto t = peek();
    if (t.type == TokenType::KEYWORD) {
        if (t.value == "let")      return parseVarDecl(false);
        if (t.value == "var")      return parseVarDecl(false);
        if (t.value == "const")    return parseVarDecl(true);
        if (t.value == "func")     return parseFuncDecl();
        if (t.value == "class")    return parseClassDecl();
        if (t.value == "return")   return parseReturnStmt();
        if (t.value == "if")       return parseIfStmt();
        if (t.value == "while")    return parseWhileStmt();
        if (t.value == "for")      return parseForStmt();
        if (t.value == "break")    return parseBreakStmt();
        if (t.value == "continue") return parseContinueStmt();
        if (t.value == "print" || t.value == "println") return parsePrintStmt();
        if (t.value == "import")   return parseImportStmt();
        if (t.value == "match")    return parseMatchStmt();
        if (t.value == "try")      return parseTryStmt();
    }
    if (t.type == TokenType::PUNCTUATION && t.value == "{")
        return parseBlock();
    return parseExprStmt();
}

ASTNodePtr Parser::parseVarDecl(bool isConst) {
    auto t = consume(); // let / var / const
    auto nameT = expect(TokenType::IDENTIFIER);
    auto node = makeNode(isConst ? NodeType::CONST_DECL : NodeType::VAR_DECL,
                         nameT.value, nameT.line, nameT.column);
    // Optional type annotation
    if (check(TokenType::PUNCTUATION, ":")) {
        consume();
        auto typeNode = makeNode(NodeType::TYPE_ANNOTATION, parseTypeName(),
                                 peek().line, peek().column);
        node->addChild(typeNode);
    }
    // Initializer
    if (match(TokenType::OPERATOR, "=")) {
        node->addChild(parseExpression());
    }
    match(TokenType::PUNCTUATION, ";");
    return node;
}

ASTNodePtr Parser::parseFuncDecl() {
    consume(); // func
    auto nameT = expect(TokenType::IDENTIFIER);
    auto node = makeNode(NodeType::FUNC_DECL, nameT.value, nameT.line, nameT.column);
    // Params
    expect(TokenType::PUNCTUATION, "(");
    if (!check(TokenType::PUNCTUATION, ")")) {
        for (auto& p : parseParams()) node->addChild(p);
    }
    expect(TokenType::PUNCTUATION, ")");
    // Return type
    if (match(TokenType::OPERATOR, "->")) {
        auto retType = makeNode(NodeType::TYPE_ANNOTATION, parseTypeName(),
                                peek().line, peek().column);
        node->addChild(retType);
    }
    node->addChild(parseBlock());
    return node;
}

ASTNodePtr Parser::parseClassDecl() {
    consume(); // class
    auto nameT = expect(TokenType::IDENTIFIER);
    auto node = makeNode(NodeType::CLASS_DECL, nameT.value, nameT.line, nameT.column);
    node->addChild(parseBlock());
    return node;
}

ASTNodePtr Parser::parseReturnStmt() {
    auto t = consume(); // return
    auto node = makeNode(NodeType::RETURN_STMT, "", t.line, t.column);
    if (!check(TokenType::PUNCTUATION, ";") && !isAtEnd())
        node->addChild(parseExpression());
    match(TokenType::PUNCTUATION, ";");
    return node;
}

ASTNodePtr Parser::parseIfStmt() {
    auto t = consume(); // if
    auto node = makeNode(NodeType::IF_STMT, "", t.line, t.column);
    node->addChild(parseExpression());
    node->addChild(parseBlock());
    if (check(TokenType::KEYWORD, "elif")) {
        node->addChild(parseIfStmt()); // recursive
    } else if (match(TokenType::KEYWORD, "else")) {
        node->addChild(parseBlock());
    }
    return node;
}

ASTNodePtr Parser::parseWhileStmt() {
    auto t = consume(); // while
    auto node = makeNode(NodeType::WHILE_STMT, "", t.line, t.column);
    node->addChild(parseExpression());
    node->addChild(parseBlock());
    return node;
}

ASTNodePtr Parser::parseForStmt() {
    auto t = consume(); // for
    auto varT = expect(TokenType::IDENTIFIER);
    expect(TokenType::KEYWORD, "in");
    auto node = makeNode(NodeType::FOR_STMT, varT.value, t.line, t.column);
    node->addChild(parseExpression());
    node->addChild(parseBlock());
    return node;
}

ASTNodePtr Parser::parseBreakStmt() {
    auto t = consume();
    match(TokenType::PUNCTUATION, ";");
    return makeNode(NodeType::BREAK_STMT, "", t.line, t.column);
}

ASTNodePtr Parser::parseContinueStmt() {
    auto t = consume();
    match(TokenType::PUNCTUATION, ";");
    return makeNode(NodeType::CONTINUE_STMT, "", t.line, t.column);
}

ASTNodePtr Parser::parsePrintStmt() {
    auto t = consume(); // print / println
    auto node = makeNode(NodeType::PRINT_STMT, t.value, t.line, t.column);
    expect(TokenType::PUNCTUATION, "(");
    if (!check(TokenType::PUNCTUATION, ")"))
        node->addChild(parseExpression());
    expect(TokenType::PUNCTUATION, ")");
    match(TokenType::PUNCTUATION, ";");
    return node;
}

ASTNodePtr Parser::parseImportStmt() {
    auto t = consume(); // import
    auto node = makeNode(NodeType::IMPORT_STMT, "", t.line, t.column);
    auto pathT = expect(TokenType::STRING);
    node->value = pathT.value;
    match(TokenType::PUNCTUATION, ";");
    return node;
}

ASTNodePtr Parser::parseMatchStmt() {
    auto t = consume(); // match
    auto node = makeNode(NodeType::MATCH_STMT, "", t.line, t.column);
    node->addChild(parseExpression());
    expect(TokenType::PUNCTUATION, "{");
    while (!isAtEnd() && !check(TokenType::PUNCTUATION, "}")) {
        auto caseNode = makeNode(NodeType::MATCH_CASE, "", peek().line, peek().column);
        if (match(TokenType::KEYWORD, "default")) {
            caseNode->value = "_";
        } else {
            expect(TokenType::KEYWORD, "case");
            caseNode->addChild(parseExpression());
        }
        expect(TokenType::OPERATOR, "=>");
        caseNode->addChild(parseBlock());
        node->addChild(caseNode);
    }
    expect(TokenType::PUNCTUATION, "}");
    return node;
}

ASTNodePtr Parser::parseTryStmt() {
    auto t = consume(); // try
    auto node = makeNode(NodeType::TRY_STMT, "", t.line, t.column);
    node->addChild(parseBlock());
    if (match(TokenType::KEYWORD, "catch")) {
        expect(TokenType::PUNCTUATION, "(");
        auto errName = expect(TokenType::IDENTIFIER);
        expect(TokenType::PUNCTUATION, ")");
        auto catchBlock = parseBlock();
        catchBlock->value = errName.value;
        node->addChild(catchBlock);
    }
    if (match(TokenType::KEYWORD, "finally")) {
        node->addChild(parseBlock());
    }
    return node;
}

ASTNodePtr Parser::parseExprStmt() {
    auto node = makeNode(NodeType::EXPR_STMT, "", peek().line, peek().column);
    node->addChild(parseExpression());
    match(TokenType::PUNCTUATION, ";");
    return node;
}

// ── Expressions (Pratt-style precedence climbing) ─────────────────────────────

ASTNodePtr Parser::parseExpression() { return parseAssignment(); }

ASTNodePtr Parser::parseAssignment() {
    auto expr = parseTernary();
    if (check(TokenType::OPERATOR, "=") || check(TokenType::OPERATOR, "+=") ||
        check(TokenType::OPERATOR, "-=") || check(TokenType::OPERATOR, "*=") ||
        check(TokenType::OPERATOR, "/=")) {
        auto op = consume();
        auto rhs = parseAssignment();
        auto node = makeNode(NodeType::ASSIGN_EXPR, op.value, op.line, op.column);
        node->addChild(expr);
        node->addChild(rhs);
        return node;
    }
    return expr;
}

ASTNodePtr Parser::parseTernary() {
    auto expr = parseOr();
    if (check(TokenType::OPERATOR, "?")) {
        consume();
        auto then = parseExpression();
        expect(TokenType::PUNCTUATION, ":");
        auto els = parseExpression();
        auto node = makeNode(NodeType::TERNARY_EXPR, "", expr->line, expr->column);
        node->addChild(expr);
        node->addChild(then);
        node->addChild(els);
        return node;
    }
    return expr;
}

ASTNodePtr Parser::parseOr() {
    auto left = parseAnd();
    while (check(TokenType::OPERATOR, "||") || check(TokenType::KEYWORD, "or")) {
        auto op = consume();
        auto right = parseAnd();
        auto node = makeNode(NodeType::BINARY_EXPR, "||", op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseAnd() {
    auto left = parseEquality();
    while (check(TokenType::OPERATOR, "&&") || check(TokenType::KEYWORD, "and")) {
        auto op = consume();
        auto right = parseEquality();
        auto node = makeNode(NodeType::BINARY_EXPR, "&&", op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenType::OPERATOR, "==") || check(TokenType::OPERATOR, "!=")) {
        auto op = consume();
        auto right = parseComparison();
        auto node = makeNode(NodeType::BINARY_EXPR, op.value, op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseComparison() {
    auto left = parseTerm();
    while (check(TokenType::OPERATOR, "<")  || check(TokenType::OPERATOR, ">") ||
           check(TokenType::OPERATOR, "<=") || check(TokenType::OPERATOR, ">=")) {
        auto op = consume();
        auto right = parseTerm();
        auto node = makeNode(NodeType::BINARY_EXPR, op.value, op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
        auto op = consume();
        auto right = parseFactor();
        auto node = makeNode(NodeType::BINARY_EXPR, op.value, op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseFactor() {
    auto left = parseUnary();
    while (check(TokenType::OPERATOR, "*") || check(TokenType::OPERATOR, "/") ||
           check(TokenType::OPERATOR, "%")) {
        auto op = consume();
        auto right = parseUnary();
        auto node = makeNode(NodeType::BINARY_EXPR, op.value, op.line, op.column);
        node->addChild(left); node->addChild(right);
        left = node;
    }
    return left;
}

ASTNodePtr Parser::parseUnary() {
    if (check(TokenType::OPERATOR, "-") || check(TokenType::OPERATOR, "!") ||
        check(TokenType::KEYWORD, "not")) {
        auto op = consume();
        auto node = makeNode(NodeType::UNARY_EXPR, op.value, op.line, op.column);
        node->addChild(parseUnary());
        return node;
    }
    return parsePostfix();
}

ASTNodePtr Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (true) {
        if (check(TokenType::PUNCTUATION, "(")) {
            expr = parseCall(expr);
        } else if (check(TokenType::OPERATOR, ".")) {
            consume();
            auto field = expect(TokenType::IDENTIFIER);
            auto node = makeNode(NodeType::MEMBER_EXPR, field.value, field.line, field.column);
            node->addChild(expr);
            expr = node;
        } else if (check(TokenType::PUNCTUATION, "[")) {
            consume();
            auto idx = parseExpression();
            expect(TokenType::PUNCTUATION, "]");
            auto node = makeNode(NodeType::INDEX_EXPR, "", expr->line, expr->column);
            node->addChild(expr); node->addChild(idx);
            expr = node;
        } else if (check(TokenType::OPERATOR, "++") || check(TokenType::OPERATOR, "--")) {
            auto op = consume();
            auto node = makeNode(NodeType::UNARY_EXPR, "post" + op.value, op.line, op.column);
            node->addChild(expr);
            expr = node;
        } else break;
    }
    return expr;
}

ASTNodePtr Parser::parsePrimary() {
    auto t = peek();
    if (t.type == TokenType::INTEGER) {
        consume();
        return makeNode(NodeType::INT_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::FLOAT) {
        consume();
        return makeNode(NodeType::FLOAT_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::STRING) {
        consume();
        return makeNode(NodeType::STRING_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::CHAR) {
        consume();
        return makeNode(NodeType::CHAR_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::BOOLEAN) {
        consume();
        return makeNode(NodeType::BOOL_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::KEYWORD && t.value == "null") {
        consume();
        return makeNode(NodeType::NULL_LITERAL, "null", t.line, t.column);
    }
    if (t.type == TokenType::KEYWORD && t.value == "true") {
        consume();
        return makeNode(NodeType::BOOL_LITERAL, "true", t.line, t.column);
    }
    if (t.type == TokenType::KEYWORD && t.value == "false") {
        consume();
        return makeNode(NodeType::BOOL_LITERAL, "false", t.line, t.column);
    }
    if (t.type == TokenType::PUNCTUATION && t.value == "[") {
        return parseListLiteral();
    }
    if (t.type == TokenType::PUNCTUATION && t.value == "{") {
        return parseMapLiteral();
    }
    if (t.type == TokenType::KEYWORD && t.value == "func") {
        return parseLambda();
    }
    if (t.type == TokenType::PUNCTUATION && t.value == "(") {
        consume();
        auto expr = parseExpression();
        expect(TokenType::PUNCTUATION, ")");
        return expr;
    }
    if (t.type == TokenType::IDENTIFIER || t.type == TokenType::KEYWORD) {
        consume();
        return makeNode(NodeType::IDENTIFIER, t.value, t.line, t.column);
    }
    error("Unexpected token: '" + t.value + "'", t.line, t.column);
    consume();
    return makeNode(NodeType::NULL_LITERAL, "null", t.line, t.column);
}

ASTNodePtr Parser::parseCall(ASTNodePtr callee) {
    auto node = makeNode(NodeType::CALL_EXPR, callee->value, callee->line, callee->column);
    node->addChild(callee);
    consume(); // (
    if (!check(TokenType::PUNCTUATION, ")")) {
        auto args = parseArgs();
        for (auto& a : args) node->addChild(a);
    }
    expect(TokenType::PUNCTUATION, ")");
    return node;
}

ASTNodePtr Parser::parseListLiteral() {
    auto t = consume(); // [
    auto node = makeNode(NodeType::LIST_LITERAL, "", t.line, t.column);
    while (!isAtEnd() && !check(TokenType::PUNCTUATION, "]")) {
        node->addChild(parseExpression());
        if (!match(TokenType::PUNCTUATION, ",")) break;
    }
    expect(TokenType::PUNCTUATION, "]");
    return node;
}

ASTNodePtr Parser::parseMapLiteral() {
    auto t = consume(); // {
    auto node = makeNode(NodeType::MAP_LITERAL, "", t.line, t.column);
    while (!isAtEnd() && !check(TokenType::PUNCTUATION, "}")) {
        auto key = parseExpression();
        expect(TokenType::PUNCTUATION, ":");
        auto val = parseExpression();
        auto pair = makeNode(NodeType::BINARY_EXPR, ":", key->line, key->column);
        pair->addChild(key); pair->addChild(val);
        node->addChild(pair);
        if (!match(TokenType::PUNCTUATION, ",")) break;
    }
    expect(TokenType::PUNCTUATION, "}");
    return node;
}

ASTNodePtr Parser::parseLambda() {
    auto t = consume(); // func
    auto node = makeNode(NodeType::LAMBDA_EXPR, "", t.line, t.column);
    expect(TokenType::PUNCTUATION, "(");
    if (!check(TokenType::PUNCTUATION, ")")) {
        for (auto& p : parseParams()) node->addChild(p);
    }
    expect(TokenType::PUNCTUATION, ")");
    if (match(TokenType::OPERATOR, "->")) parseTypeName();
    node->addChild(parseBlock());
    return node;
}

std::string Parser::parseTypeName() {
    std::string name;
    auto t = peek();
    if (t.type == TokenType::KEYWORD || t.type == TokenType::IDENTIFIER) {
        name = t.value; consume();
        if (check(TokenType::OPERATOR, "<")) {
            consume(); name += "<";
            name += parseTypeName(); name += ">";
            expect(TokenType::OPERATOR, ">");
        }
    }
    return name.empty() ? "auto" : name;
}

std::vector<ASTNodePtr> Parser::parseParams() {
    std::vector<ASTNodePtr> params;
    do {
        auto nameT = expect(TokenType::IDENTIFIER);
        auto param = makeNode(NodeType::PARAM, nameT.value, nameT.line, nameT.column);
        if (match(TokenType::PUNCTUATION, ":")) {
            param->addChild(makeNode(NodeType::TYPE_ANNOTATION, parseTypeName(),
                                     peek().line, peek().column));
        }
        if (match(TokenType::OPERATOR, "=")) {
            param->addChild(parseExpression());
        }
        params.push_back(param);
    } while (match(TokenType::PUNCTUATION, ",") && !check(TokenType::PUNCTUATION, ")"));
    return params;
}

std::vector<ASTNodePtr> Parser::parseArgs() {
    std::vector<ASTNodePtr> args;
    do {
        args.push_back(parseExpression());
    } while (match(TokenType::PUNCTUATION, ",") && !check(TokenType::PUNCTUATION, ")"));
    return args;
}
