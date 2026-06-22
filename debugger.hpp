// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "ast.hpp"
#include "error_handler.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>

class Debugger {
public:
    Debugger(ErrorHandler& err) : err_(err), enabled_(false), stepMode_(false) {}

    void enable()  { enabled_ = true; }
    void disable() { enabled_ = false; }
    bool isEnabled() const { return enabled_; }

    void setStepMode(bool on) { stepMode_ = on; }

    void addBreakpoint(const std::string& file, int line) {
        breakpoints_.insert(file + ":" + std::to_string(line));
    }

    void removeBreakpoint(const std::string& file, int line) {
        breakpoints_.erase(file + ":" + std::to_string(line));
    }

    bool shouldBreak(const std::string& file, int line) const {
        if (!enabled_) return false;
        if (stepMode_) return true;
        return breakpoints_.count(file + ":" + std::to_string(line)) > 0;
    }

    void onEnterNode(ASTNodePtr n, const std::string& file = "") {
        if (!enabled_ || !n) return;
        if (shouldBreak(file, n->line)) {
            std::cout << "\n[Debugger] Paused at " << file << ":" << n->line << "\n";
            std::cout << "  Node: " << nodeTypeName(n->type) << " [" << n->value << "]\n";
            waitForCommand();
        }
    }

    void printAST(ASTNodePtr root, int depth = 0) const {
        if (!root) return;
        std::string pad(depth * 2, ' ');
        std::cout << pad << nodeTypeName(root->type);
        if (!root->value.empty()) std::cout << " [" << root->value << "]";
        std::cout << " (" << root->line << ":" << root->column << ")\n";
        for (auto& c : root->children) printAST(c, depth + 1);
    }

    static std::string nodeTypeName(NodeType t) {
        switch (t) {
            case NodeType::PROGRAM:      return "Program";
            case NodeType::BLOCK:        return "Block";
            case NodeType::VAR_DECL:     return "VarDecl";
            case NodeType::CONST_DECL:   return "ConstDecl";
            case NodeType::FUNC_DECL:    return "FuncDecl";
            case NodeType::CLASS_DECL:   return "ClassDecl";
            case NodeType::RETURN_STMT:  return "ReturnStmt";
            case NodeType::IF_STMT:      return "IfStmt";
            case NodeType::WHILE_STMT:   return "WhileStmt";
            case NodeType::FOR_STMT:     return "ForStmt";
            case NodeType::BREAK_STMT:   return "BreakStmt";
            case NodeType::CONTINUE_STMT:return "ContinueStmt";
            case NodeType::EXPR_STMT:    return "ExprStmt";
            case NodeType::PRINT_STMT:   return "PrintStmt";
            case NodeType::BINARY_EXPR:  return "BinaryExpr";
            case NodeType::UNARY_EXPR:   return "UnaryExpr";
            case NodeType::ASSIGN_EXPR:  return "AssignExpr";
            case NodeType::CALL_EXPR:    return "CallExpr";
            case NodeType::INT_LITERAL:  return "IntLiteral";
            case NodeType::FLOAT_LITERAL:return "FloatLiteral";
            case NodeType::STRING_LITERAL:return "StringLiteral";
            case NodeType::BOOL_LITERAL: return "BoolLiteral";
            case NodeType::IDENTIFIER:   return "Identifier";
            default:                     return "Node";
        }
    }

private:
    ErrorHandler& err_;
    bool enabled_;
    bool stepMode_;
    std::set<std::string> breakpoints_;

    void waitForCommand() {
        std::cout << "(app-debug) ";
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd == "c" || cmd == "continue") { stepMode_ = false; }
        else if (cmd == "s" || cmd == "step") { stepMode_ = true; }
        else if (cmd == "q" || cmd == "quit") { exit(0); }
    }
};

#endif // DEBUGGER_HPP
