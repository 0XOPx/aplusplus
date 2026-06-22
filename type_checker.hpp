// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "ast.hpp"
#include "symbol_table.hpp"
#include "error_handler.hpp"
#include <string>
#include <map>

class TypeChecker {
public:
    TypeChecker(ErrorHandler& errHandler)
        : err_(errHandler) {
        scope_ = std::make_shared<SymbolTable>();
        initBuiltins();
    }

    void check(ASTNodePtr node) {
        if (!node) return;
        switch (node->type) {
            case NodeType::PROGRAM:
            case NodeType::BLOCK:
                for (auto& child : node->children) check(child);
                break;
            case NodeType::VAR_DECL:
            case NodeType::CONST_DECL:
                checkVarDecl(node);
                break;
            case NodeType::FUNC_DECL:
                checkFuncDecl(node);
                break;
            case NodeType::IF_STMT:
                for (auto& child : node->children) check(child);
                break;
            case NodeType::WHILE_STMT:
            case NodeType::FOR_STMT:
                for (auto& child : node->children) check(child);
                break;
            default:
                for (auto& child : node->children) check(child);
                break;
        }
    }

private:
    ErrorHandler& err_;
    std::shared_ptr<SymbolTable> scope_;

    void initBuiltins() {
        Symbol printSym{"print", "void", SymbolKind::FUNCTION, false, true};
        Symbol printlnSym{"println", "void", SymbolKind::FUNCTION, false, true};
        Symbol inputSym{"input", "string", SymbolKind::FUNCTION, false, true};
        try { scope_->define(printSym); } catch (...) {}
        try { scope_->define(printlnSym); } catch (...) {}
        try { scope_->define(inputSym); } catch (...) {}
    }

    void checkVarDecl(ASTNodePtr node) {
        Symbol sym;
        sym.name = node->value;
        sym.kind = (node->type == NodeType::CONST_DECL) ? SymbolKind::CONSTANT : SymbolKind::VARIABLE;
        sym.isConst = (node->type == NodeType::CONST_DECL);
        sym.typeName = node->children.size() > 0 ? node->children[0]->value : "auto";
        sym.isInitialized = node->children.size() > 1;
        sym.line = node->line;
        try { scope_->define(sym); }
        catch (const std::exception& e) {
            err_.report(ErrorCode::NAME_ERROR, e.what(), node->line, node->column);
        }
    }

    void checkFuncDecl(ASTNodePtr node) {
        Symbol sym;
        sym.name = node->value;
        sym.kind = SymbolKind::FUNCTION;
        sym.typeName = "func";
        sym.isInitialized = true;
        sym.line = node->line;
        try { scope_->define(sym); } catch (...) {}
        auto prev = scope_;
        scope_ = std::make_shared<SymbolTable>();
        for (auto& child : node->children) check(child);
        scope_ = prev;
    }

    std::string inferType(ASTNodePtr node) {
        if (!node) return "void";
        switch (node->type) {
            case NodeType::INT_LITERAL:    return "int";
            case NodeType::FLOAT_LITERAL:  return "float";
            case NodeType::STRING_LITERAL: return "string";
            case NodeType::BOOL_LITERAL:   return "bool";
            case NodeType::CHAR_LITERAL:   return "char";
            case NodeType::NULL_LITERAL:   return "null";
            default: return "auto";
        }
    }
};

#endif // TYPE_CHECKER_HPP
