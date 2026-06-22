// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "ast.hpp"
#include "error_handler.hpp"
#include <memory>
#include <string>

class Optimizer {
public:
    Optimizer(int level, ErrorHandler& err) : level_(level), err_(err) {}

    ASTNodePtr optimize(ASTNodePtr root) {
        if (!root || level_ == 0) return root;
        return foldConstants(root);
    }

private:
    int           level_;
    ErrorHandler& err_;

    ASTNodePtr foldConstants(ASTNodePtr n) {
        if (!n) return n;
        for (auto& child : n->children) child = foldConstants(child);

        if (n->type == NodeType::BINARY_EXPR && n->children.size() == 2) {
            auto& L = n->children[0];
            auto& R = n->children[1];
            if (L->type == NodeType::INT_LITERAL && R->type == NodeType::INT_LITERAL) {
                long long lv = std::stoll(L->value);
                long long rv = std::stoll(R->value);
                long long result = 0;
                bool folded = true;
                const std::string& op = n->value;
                if (op == "+")  result = lv + rv;
                else if (op == "-") result = lv - rv;
                else if (op == "*") result = lv * rv;
                else if (op == "/" && rv != 0) result = lv / rv;
                else if (op == "%" && rv != 0) result = lv % rv;
                else folded = false;
                if (folded) {
                    auto node = makeNode(NodeType::INT_LITERAL, std::to_string(result), n->line, n->column);
                    return node;
                }
            }
            if (L->type == NodeType::FLOAT_LITERAL && R->type == NodeType::FLOAT_LITERAL) {
                double lv = std::stod(L->value);
                double rv = std::stod(R->value);
                const std::string& op = n->value;
                double result = 0;
                bool folded = true;
                if (op == "+")  result = lv + rv;
                else if (op == "-") result = lv - rv;
                else if (op == "*") result = lv * rv;
                else if (op == "/" && rv != 0) result = lv / rv;
                else folded = false;
                if (folded) {
                    auto node = makeNode(NodeType::FLOAT_LITERAL, std::to_string(result), n->line, n->column);
                    return node;
                }
            }
            // String concatenation folding
            if (n->value == "+" &&
                L->type == NodeType::STRING_LITERAL &&
                R->type == NodeType::STRING_LITERAL) {
                return makeNode(NodeType::STRING_LITERAL, L->value + R->value, n->line, n->column);
            }
        }

        // Dead-code elimination: if (false) { ... }
        if (n->type == NodeType::IF_STMT && !n->children.empty()) {
            auto& cond = n->children[0];
            if (cond->type == NodeType::BOOL_LITERAL) {
                if (cond->value == "false" && n->children.size() > 2)
                    return n->children[2]; // else branch
                if (cond->value == "true" && n->children.size() > 1)
                    return n->children[1]; // then branch
                return makeNode(NodeType::BLOCK, "", n->line, n->column);
            }
        }

        return n;
    }
};

#endif // OPTIMIZER_HPP
