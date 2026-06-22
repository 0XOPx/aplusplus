// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    PROGRAM,
    BLOCK,
    // Statements
    VAR_DECL,
    CONST_DECL,
    FUNC_DECL,
    CLASS_DECL,
    RETURN_STMT,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    BREAK_STMT,
    CONTINUE_STMT,
    EXPR_STMT,
    IMPORT_STMT,
    PRINT_STMT,
    // Expressions
    BINARY_EXPR,
    UNARY_EXPR,
    ASSIGN_EXPR,
    CALL_EXPR,
    MEMBER_EXPR,
    INDEX_EXPR,
    TERNARY_EXPR,
    LAMBDA_EXPR,
    // Literals
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,
    CHAR_LITERAL,
    NULL_LITERAL,
    LIST_LITERAL,
    MAP_LITERAL,
    // Identifiers & Types
    IDENTIFIER,
    TYPE_ANNOTATION,
    PARAM,
    MATCH_STMT,
    MATCH_CASE,
    TRY_STMT,
    THROW_STMT
};

struct ASTNode {
    NodeType type;
    std::string value;
    int line;
    int column;
    std::vector<std::shared_ptr<ASTNode>> children;

    ASTNode(NodeType t, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}

    void addChild(std::shared_ptr<ASTNode> child) {
        children.push_back(std::move(child));
    }
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

inline ASTNodePtr makeNode(NodeType t, const std::string& v = "", int l = 0, int c = 0) {
    return std::make_shared<ASTNode>(t, v, l, c);
}

#endif // AST_HPP
