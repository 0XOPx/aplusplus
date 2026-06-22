// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>

enum class SymbolKind { VARIABLE, FUNCTION, CLASS, PARAMETER, CONSTANT };

struct Symbol {
    std::string name;
    std::string typeName;
    SymbolKind  kind;
    bool        isConst = false;
    bool        isInitialized = false;
    int         line = 0;
};

class SymbolTable {
public:
    SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr)
        : parent_(parent) {}

    void define(const Symbol& sym) {
        if (table_.count(sym.name))
            throw std::runtime_error("Symbol already defined: " + sym.name);
        table_[sym.name] = sym;
    }

    Symbol& resolve(const std::string& name) {
        auto it = table_.find(name);
        if (it != table_.end()) return it->second;
        if (parent_) return parent_->resolve(name);
        throw std::runtime_error("Undefined symbol: " + name);
    }

    bool has(const std::string& name) const {
        if (table_.count(name)) return true;
        if (parent_) return parent_->has(name);
        return false;
    }

    bool hasLocal(const std::string& name) const {
        return table_.count(name) > 0;
    }

    std::shared_ptr<SymbolTable> parent() const { return parent_; }
    std::shared_ptr<SymbolTable> makeChild() {
        return std::make_shared<SymbolTable>(shared_from_this_helper());
    }

private:
    std::map<std::string, Symbol> table_;
    std::shared_ptr<SymbolTable>  parent_;

    std::shared_ptr<SymbolTable> shared_from_this_helper() {
        return std::make_shared<SymbolTable>(
            std::shared_ptr<SymbolTable>(this, [](SymbolTable*){}));
    }
};

#endif // SYMBOL_TABLE_HPP
