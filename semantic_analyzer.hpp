// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "ast.hpp"
#include "symbol_table.hpp"
#include "error_handler.hpp"
#include <string>
#include <memory>

class SemanticAnalyzer {
public:
    SemanticAnalyzer(ErrorHandler& err)
        : err_(err) {
        global_ = std::make_shared<SymbolTable>();
        current_ = global_;
        registerBuiltins();
    }

    void analyze(ASTNodePtr node) {
        if (!node) return;
        visit(node);
    }

private:
    ErrorHandler& err_;
    std::shared_ptr<SymbolTable> global_;
    std::shared_ptr<SymbolTable> current_;
    std::string currentFuncReturnType_;

    void registerBuiltins() {
        auto reg = [&](const std::string& name) {
            Symbol s{name, "builtin", SymbolKind::FUNCTION, false, true, 0};
            try { global_->define(s); } catch (...) {}
        };
        reg("print"); reg("println"); reg("input");
        reg("len"); reg("range"); reg("str"); reg("int"); reg("float");
        reg("list"); reg("map"); reg("push"); reg("pop"); reg("keys");
    }

    void visit(ASTNodePtr n) {
        if (!n) return;
        switch (n->type) {
            case NodeType::PROGRAM:
            case NodeType::BLOCK:
                for (auto& c : n->children) visit(c);
                break;
            case NodeType::VAR_DECL:
            case NodeType::CONST_DECL:
                visitVarDecl(n);
                break;
            case NodeType::FUNC_DECL:
                visitFuncDecl(n);
                break;
            case NodeType::CLASS_DECL:
                visitClassDecl(n);
                break;
            case NodeType::IDENTIFIER:
                checkIdentifierDefined(n);
                break;
            case NodeType::RETURN_STMT:
                for (auto& c : n->children) visit(c);
                break;
            default:
                for (auto& c : n->children) visit(c);
                break;
        }
    }

    void visitVarDecl(ASTNodePtr n) {
        Symbol sym;
        sym.name = n->value;
        sym.kind = (n->type == NodeType::CONST_DECL) ? SymbolKind::CONSTANT : SymbolKind::VARIABLE;
        sym.isConst = (n->type == NodeType::CONST_DECL);
        sym.typeName = "auto";
        sym.isInitialized = !n->children.empty();
        sym.line = n->line;
        if (!n->children.empty()) visit(n->children.back());
        try { current_->define(sym); }
        catch (const std::exception& e) {
            err_.report(ErrorCode::NAME_ERROR, e.what(), n->line, n->column);
        }
    }

    void visitFuncDecl(ASTNodePtr n) {
        Symbol sym{n->value, "func", SymbolKind::FUNCTION, false, true, n->line};
        try { current_->define(sym); } catch (...) {}
        auto outer = current_;
        current_ = std::make_shared<SymbolTable>();
        for (auto& c : n->children) visit(c);
        current_ = outer;
    }

    void visitClassDecl(ASTNodePtr n) {
        Symbol sym{n->value, "class", SymbolKind::CLASS, false, true, n->line};
        try { current_->define(sym); } catch (...) {}
        auto outer = current_;
        current_ = std::make_shared<SymbolTable>();
        for (auto& c : n->children) visit(c);
        current_ = outer;
    }

    void checkIdentifierDefined(ASTNodePtr n) {
        if (!current_->has(n->value)) {
            err_.report(ErrorCode::NAME_ERROR,
                "Undefined identifier: '" + n->value + "'",
                n->line, n->column);
        }
    }
};

#endif // SEMANTIC_ANALYZER_HPP
