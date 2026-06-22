// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "error.hpp"
#include <vector>
#include <string>
#include <iostream>

class ErrorHandler {
public:
    ErrorHandler() : hasErrors_(false), hasFatalError_(false) {}

    void report(ErrorCode code, const std::string& message,
                int line = 0, int column = 0, const std::string& filename = "") {
        AppError err(code, message, line, column, filename);
        errors_.push_back(err);
        hasErrors_ = true;
        std::cerr << err.format() << std::endl;
    }

    void reportFatal(ErrorCode code, const std::string& message,
                     int line = 0, int column = 0, const std::string& filename = "") {
        hasFatalError_ = true;
        report(code, message, line, column, filename);
        throw errors_.back();
    }

    void warn(const std::string& message, int line = 0, const std::string& filename = "") {
        std::string loc = filename.empty() ? "" : filename + ":";
        if (line > 0) loc += std::to_string(line) + ": ";
        std::cerr << "\033[33mWarning\033[0m: " << loc << message << std::endl;
    }

    bool hasErrors() const { return hasErrors_; }
    bool hasFatalError() const { return hasFatalError_; }
    int errorCount() const { return static_cast<int>(errors_.size()); }
    const std::vector<AppError>& errors() const { return errors_; }

    void clear() {
        errors_.clear();
        hasErrors_ = false;
        hasFatalError_ = false;
    }

private:
    std::vector<AppError> errors_;
    bool hasErrors_;
    bool hasFatalError_;
};

#endif // ERROR_HANDLER_HPP
