// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "ast.hpp"
#include "error_handler.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <variant>
#include <functional>
#include <iostream>
#include <stdexcept>

struct AppValue;
using AppValuePtr = std::shared_ptr<AppValue>;
using NativeFunc = std::function<AppValuePtr(std::vector<AppValuePtr>)>;

struct AppValue {
    enum class Kind { INT, FLOAT, STRING, BOOL, CHAR, NIL, LIST, MAP, FUNC, NATIVE } kind;
    long long   ival = 0;
    double      fval = 0.0;
    std::string sval;
    bool        bval = false;
    std::vector<AppValuePtr> list;
    std::map<std::string, AppValuePtr> mmap;
    NativeFunc  native;
    ASTNodePtr  funcNode;
    std::string typeName = "nil";

    static AppValuePtr makeInt(long long v)    { auto p = std::make_shared<AppValue>(); p->kind=Kind::INT;    p->ival=v;    p->typeName="int";    return p; }
    static AppValuePtr makeFloat(double v)     { auto p = std::make_shared<AppValue>(); p->kind=Kind::FLOAT;  p->fval=v;    p->typeName="float";  return p; }
    static AppValuePtr makeString(const std::string& v){ auto p=std::make_shared<AppValue>(); p->kind=Kind::STRING; p->sval=v; p->typeName="string"; return p; }
    static AppValuePtr makeBool(bool v)        { auto p = std::make_shared<AppValue>(); p->kind=Kind::BOOL;   p->bval=v;    p->typeName="bool";   return p; }
    static AppValuePtr makeChar(char v)        { auto p = std::make_shared<AppValue>(); p->kind=Kind::CHAR;   p->sval=std::string(1,v); p->typeName="char"; return p; }
    static AppValuePtr makeNil()               { auto p = std::make_shared<AppValue>(); p->kind=Kind::NIL;    p->typeName="null";  return p; }
    static AppValuePtr makeList(std::vector<AppValuePtr> v){ auto p=std::make_shared<AppValue>(); p->kind=Kind::LIST; p->list=v; p->typeName="list"; return p; }
    static AppValuePtr makeNative(NativeFunc f){ auto p=std::make_shared<AppValue>(); p->kind=Kind::NATIVE; p->native=f; p->typeName="func"; return p; }

    std::string toString() const {
        switch(kind){
            case Kind::INT:    return std::to_string(ival);
            case Kind::FLOAT:  return std::to_string(fval);
            case Kind::STRING: return sval;
            case Kind::BOOL:   return bval ? "true" : "false";
            case Kind::CHAR:   return sval;
            case Kind::NIL:    return "null";
            case Kind::LIST: {
                std::string s = "[";
                for(size_t i=0;i<list.size();i++){if(i>0)s+=", ";s+=list[i]->toString();}
                return s + "]";
            }
            default: return "<func>";
        }
    }

    bool isTruthy() const {
        switch(kind){
            case Kind::BOOL:   return bval;
            case Kind::INT:    return ival != 0;
            case Kind::FLOAT:  return fval != 0.0;
            case Kind::STRING: return !sval.empty();
            case Kind::NIL:    return false;
            default:           return true;
        }
    }
};

struct Environment {
    std::map<std::string, AppValuePtr> vars;
    std::shared_ptr<Environment> parent;

    Environment(std::shared_ptr<Environment> p = nullptr) : parent(p) {}

    void set(const std::string& name, AppValuePtr val) { vars[name] = val; }

    void assign(const std::string& name, AppValuePtr val) {
        if (vars.count(name)) { vars[name] = val; return; }
        if (parent) { parent->assign(name, val); return; }
        throw std::runtime_error("Undefined variable: " + name);
    }

    AppValuePtr get(const std::string& name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return it->second;
        if (parent) return parent->get(name);
        throw std::runtime_error("Undefined variable: '" + name + "'");
    }

    bool has(const std::string& name) const {
        if (vars.count(name)) return true;
        if (parent) return parent->has(name);
        return false;
    }
};

struct ReturnSignal { AppValuePtr value; };
struct BreakSignal {};
struct ContinueSignal {};

class Interpreter {
public:
    Interpreter(ErrorHandler& err) : err_(err) {
        global_ = std::make_shared<Environment>();
        registerBuiltins();
    }

    void run(ASTNodePtr root) {
        try { exec(root, global_); }
        catch (const AppError& e) { printError(e); }
        catch (const std::exception& e) {
            err_.report(ErrorCode::RUNTIME_ERROR, e.what());
        }
    }

    AppValuePtr eval(ASTNodePtr node) { return evalExpr(node, global_); }

private:
    ErrorHandler& err_;
    std::shared_ptr<Environment> global_;

    void registerBuiltins() {
        global_->set("print", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            for (auto& a : args) std::cout << a->toString();
            return AppValue::makeNil();
        }));
        global_->set("println", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            for (auto& a : args) std::cout << a->toString();
            std::cout << std::endl;
            return AppValue::makeNil();
        }));
        global_->set("input", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            if (!args.empty()) std::cout << args[0]->toString();
            std::string line; std::getline(std::cin, line);
            return AppValue::makeString(line);
        }));
        global_->set("len", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            if (args.empty()) return AppValue::makeInt(0);
            if (args[0]->kind == AppValue::Kind::LIST) return AppValue::makeInt(args[0]->list.size());
            if (args[0]->kind == AppValue::Kind::STRING) return AppValue::makeInt(args[0]->sval.size());
            return AppValue::makeInt(0);
        }));
        global_->set("str", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            if (args.empty()) return AppValue::makeString("");
            return AppValue::makeString(args[0]->toString());
        }));
        global_->set("int", AppValue::makeNative([](std::vector<AppValuePtr> args) -> AppValuePtr {
            if (args.empty()) return AppValue::makeInt(0);
            if (args[0]->kind == AppValue::Kind::INT) return args[0];
            if (args[0]->kind == AppValue::Kind::FLOAT) return AppValue::makeInt((long long)args[0]->fval);
            if (args[0]->kind == AppValue::Kind::STRING) return AppValue::makeInt(std::stoll(args[0]->sval));
            return AppValue::makeInt(0);
        }));
    }

    void exec(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (!n) return;
        switch (n->type) {
            case NodeType::PROGRAM:
            case NodeType::BLOCK:
                for (auto& c : n->children) exec(c, env);
                break;
            case NodeType::VAR_DECL:
            case NodeType::CONST_DECL: {
                AppValuePtr val = AppValue::makeNil();
                if (!n->children.empty()) val = evalExpr(n->children.back(), env);
                env->set(n->value, val);
                break;
            }
            case NodeType::FUNC_DECL: {
                auto fn = AppValue::makeNative(nullptr);
                fn->kind = AppValue::Kind::FUNC;
                fn->funcNode = n;
                env->set(n->value, fn);
                break;
            }
            case NodeType::EXPR_STMT:
                evalExpr(n->children.empty() ? n : n->children[0], env);
                break;
            case NodeType::PRINT_STMT: {
                AppValuePtr val = AppValue::makeNil();
                if (!n->children.empty()) val = evalExpr(n->children[0], env);
                std::cout << val->toString() << std::endl;
                break;
            }
            case NodeType::RETURN_STMT: {
                AppValuePtr val = AppValue::makeNil();
                if (!n->children.empty()) val = evalExpr(n->children[0], env);
                throw ReturnSignal{val};
            }
            case NodeType::BREAK_STMT:   throw BreakSignal{};
            case NodeType::CONTINUE_STMT: throw ContinueSignal{};
            case NodeType::IF_STMT:      execIf(n, env); break;
            case NodeType::WHILE_STMT:   execWhile(n, env); break;
            case NodeType::FOR_STMT:     execFor(n, env); break;
            default:
                evalExpr(n, env);
                break;
        }
    }

    void execIf(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (n->children.empty()) return;
        auto cond = evalExpr(n->children[0], env);
        if (cond->isTruthy()) {
            if (n->children.size() > 1) exec(n->children[1], env);
        } else {
            if (n->children.size() > 2) exec(n->children[2], env);
        }
    }

    void execWhile(ASTNodePtr n, std::shared_ptr<Environment> env) {
        while (true) {
            if (n->children.empty()) break;
            auto cond = evalExpr(n->children[0], env);
            if (!cond->isTruthy()) break;
            try {
                if (n->children.size() > 1) exec(n->children[1], env);
            } catch (BreakSignal&) { break; }
              catch (ContinueSignal&) { continue; }
        }
    }

    void execFor(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (n->children.size() < 2) return;
        auto iterable = evalExpr(n->children[1], env);
        auto inner = std::make_shared<Environment>(env);
        for (auto& item : iterable->list) {
            inner->set(n->value, item);
            try { exec(n->children.size() > 2 ? n->children[2] : n->children[1], inner); }
            catch (BreakSignal&) { break; }
            catch (ContinueSignal&) { continue; }
        }
    }

    AppValuePtr evalExpr(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (!n) return AppValue::makeNil();
        switch (n->type) {
            case NodeType::INT_LITERAL:    return AppValue::makeInt(std::stoll(n->value));
            case NodeType::FLOAT_LITERAL:  return AppValue::makeFloat(std::stod(n->value));
            case NodeType::STRING_LITERAL: return AppValue::makeString(n->value);
            case NodeType::BOOL_LITERAL:   return AppValue::makeBool(n->value == "true");
            case NodeType::CHAR_LITERAL:   return AppValue::makeChar(n->value.empty() ? '\0' : n->value[0]);
            case NodeType::NULL_LITERAL:   return AppValue::makeNil();
            case NodeType::IDENTIFIER:
                try { return env->get(n->value); }
                catch (const std::exception& e) {
                    err_.report(ErrorCode::NAME_ERROR, e.what(), n->line, n->column);
                    return AppValue::makeNil();
                }
            case NodeType::ASSIGN_EXPR: {
                auto val = evalExpr(n->children.back(), env);
                if (!n->children.empty()) {
                    try { env->assign(n->value, val); }
                    catch (...) { env->set(n->value, val); }
                }
                return val;
            }
            case NodeType::BINARY_EXPR:  return evalBinary(n, env);
            case NodeType::UNARY_EXPR:   return evalUnary(n, env);
            case NodeType::CALL_EXPR:    return evalCall(n, env);
            case NodeType::LIST_LITERAL: {
                std::vector<AppValuePtr> items;
                for (auto& c : n->children) items.push_back(evalExpr(c, env));
                return AppValue::makeList(items);
            }
            default:
                for (auto& c : n->children) evalExpr(c, env);
                return AppValue::makeNil();
        }
    }

    AppValuePtr evalBinary(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (n->children.size() < 2) return AppValue::makeNil();
        auto left  = evalExpr(n->children[0], env);
        auto right = evalExpr(n->children[1], env);
        const std::string& op = n->value;

        if (op == "+") {
            if (left->kind == AppValue::Kind::STRING || right->kind == AppValue::Kind::STRING)
                return AppValue::makeString(left->toString() + right->toString());
            if (left->kind == AppValue::Kind::FLOAT || right->kind == AppValue::Kind::FLOAT)
                return AppValue::makeFloat(toDouble(left) + toDouble(right));
            return AppValue::makeInt(left->ival + right->ival);
        }
        if (op == "-") {
            if (left->kind == AppValue::Kind::FLOAT || right->kind == AppValue::Kind::FLOAT)
                return AppValue::makeFloat(toDouble(left) - toDouble(right));
            return AppValue::makeInt(left->ival - right->ival);
        }
        if (op == "*") {
            if (left->kind == AppValue::Kind::FLOAT || right->kind == AppValue::Kind::FLOAT)
                return AppValue::makeFloat(toDouble(left) * toDouble(right));
            return AppValue::makeInt(left->ival * right->ival);
        }
        if (op == "/") {
            double r = toDouble(right);
            if (r == 0.0) { err_.report(ErrorCode::DIVISION_BY_ZERO, "Division by zero", n->line); return AppValue::makeNil(); }
            return AppValue::makeFloat(toDouble(left) / r);
        }
        if (op == "%") {
            if (right->ival == 0) { err_.report(ErrorCode::DIVISION_BY_ZERO, "Modulo by zero", n->line); return AppValue::makeNil(); }
            return AppValue::makeInt(left->ival % right->ival);
        }
        if (op == "==") return AppValue::makeBool(left->toString() == right->toString());
        if (op == "!=") return AppValue::makeBool(left->toString() != right->toString());
        if (op == "<")  return AppValue::makeBool(toDouble(left) < toDouble(right));
        if (op == ">")  return AppValue::makeBool(toDouble(left) > toDouble(right));
        if (op == "<=") return AppValue::makeBool(toDouble(left) <= toDouble(right));
        if (op == ">=") return AppValue::makeBool(toDouble(left) >= toDouble(right));
        if (op == "&&" || op == "and") return AppValue::makeBool(left->isTruthy() && right->isTruthy());
        if (op == "||" || op == "or")  return AppValue::makeBool(left->isTruthy() || right->isTruthy());
        return AppValue::makeNil();
    }

    AppValuePtr evalUnary(ASTNodePtr n, std::shared_ptr<Environment> env) {
        if (n->children.empty()) return AppValue::makeNil();
        auto val = evalExpr(n->children[0], env);
        if (n->value == "-") {
            if (val->kind == AppValue::Kind::FLOAT) return AppValue::makeFloat(-val->fval);
            return AppValue::makeInt(-val->ival);
        }
        if (n->value == "!" || n->value == "not") return AppValue::makeBool(!val->isTruthy());
        return val;
    }

    AppValuePtr evalCall(ASTNodePtr n, std::shared_ptr<Environment> env) {
        std::vector<AppValuePtr> args;
        for (size_t i = 1; i < n->children.size(); i++)
            args.push_back(evalExpr(n->children[i], env));

        AppValuePtr callee;
        try { callee = env->get(n->value); }
        catch (...) {
            err_.report(ErrorCode::NAME_ERROR, "Undefined function: '" + n->value + "'", n->line);
            return AppValue::makeNil();
        }

        if (callee->kind == AppValue::Kind::NATIVE && callee->native)
            return callee->native(args);

        if (callee->kind == AppValue::Kind::FUNC && callee->funcNode) {
            auto funcEnv = std::make_shared<Environment>(global_);
            auto fn = callee->funcNode;
            try {
                for (auto& c : fn->children) exec(c, funcEnv);
            } catch (ReturnSignal& ret) { return ret.value; }
            return AppValue::makeNil();
        }

        err_.report(ErrorCode::TYPE_ERROR, "'" + n->value + "' is not callable", n->line);
        return AppValue::makeNil();
    }

    double toDouble(AppValuePtr v) const {
        if (v->kind == AppValue::Kind::FLOAT) return v->fval;
        if (v->kind == AppValue::Kind::INT)   return (double)v->ival;
        if (v->kind == AppValue::Kind::BOOL)  return v->bval ? 1.0 : 0.0;
        return 0.0;
    }
};

#endif // INTERPRETER_HPP
