// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <stdexcept>
#include <iostream>

enum class ErrorCode {
    SYNTAX_ERROR,
    TYPE_ERROR,
    RUNTIME_ERROR,
    NAME_ERROR,
    INDEX_ERROR,
    DIVISION_BY_ZERO,
    IMPORT_ERROR,
    FILE_ERROR,
    INTERNAL_ERROR
};

struct AppError : public std::runtime_error {
    ErrorCode code;
    int line;
    int column;
    std::string filename;

    AppError(ErrorCode c, const std::string& msg,
             int l = 0, int col = 0, const std::string& f = "")
        : std::runtime_error(msg), code(c), line(l), column(col), filename(f) {}

    std::string format() const {
        std::string prefix;
        switch (code) {
            case ErrorCode::SYNTAX_ERROR:    prefix = "SyntaxError";    break;
            case ErrorCode::TYPE_ERROR:      prefix = "TypeError";      break;
            case ErrorCode::RUNTIME_ERROR:   prefix = "RuntimeError";   break;
            case ErrorCode::NAME_ERROR:      prefix = "NameError";      break;
            case ErrorCode::INDEX_ERROR:     prefix = "IndexError";     break;
            case ErrorCode::DIVISION_BY_ZERO:prefix = "ZeroDivisionError"; break;
            case ErrorCode::IMPORT_ERROR:    prefix = "ImportError";    break;
            case ErrorCode::FILE_ERROR:      prefix = "FileError";      break;
            default:                         prefix = "InternalError";  break;
        }

        std::string loc = "";
        if (!filename.empty()) loc += filename + ":";
        if (line > 0) loc += std::to_string(line);
        if (column > 0) loc += ":" + std::to_string(column);
        if (!loc.empty()) loc = " at " + loc;

        return "\033[31m" + prefix + loc + "\033[0m: " + what();
    }
};

inline void printError(const AppError& e) {
    std::cerr << e.format() << std::endl;
}

inline void printError(const std::string& msg) {
    std::cerr << "\033[31mError\033[0m: " << msg << std::endl;
}

#endif // ERROR_HPP
