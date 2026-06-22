// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "interpreter.hpp"
#include "error_handler.hpp"
#include "config.hpp"
#include <string>
#include <iostream>

class Runtime {
public:
    Runtime(const Config& cfg, ErrorHandler& err)
        : cfg_(cfg), err_(err), interp_(err) {}

    int run(ASTNodePtr ast) {
        if (!ast) return 1;
        try {
            interp_.run(ast);
            return 0;
        } catch (const AppError& e) {
            printError(e);
            return 1;
        } catch (const std::exception& e) {
            err_.report(ErrorCode::RUNTIME_ERROR, e.what());
            return 1;
        }
    }

    AppValuePtr eval(ASTNodePtr expr) {
        return interp_.eval(expr);
    }

    Interpreter& interpreter() { return interp_; }

private:
    const Config& cfg_;
    ErrorHandler& err_;
    Interpreter   interp_;
};

#endif // RUNTIME_HPP
