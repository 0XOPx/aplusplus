// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "error_handler.hpp"
#include "utils.hpp"
#include <string>
#include <map>
#include <vector>
#include <sstream>

class Preprocessor {
public:
    Preprocessor(ErrorHandler& err) : err_(err) {}

    std::string process(const std::string& source, const std::string& filename = "") {
        filename_ = filename;
        std::string result;
        std::istringstream stream(source);
        std::string line;
        int lineNum = 0;

        while (std::getline(stream, line)) {
            lineNum++;
            std::string trimmed = Utils::trim(line);

            if (Utils::startsWith(trimmed, "#define ")) {
                processDefine(trimmed.substr(8), lineNum);
            } else if (Utils::startsWith(trimmed, "#ifdef ") || Utils::startsWith(trimmed, "#ifndef ")) {
                // simple passthrough for now
                result += line + "\n";
            } else if (Utils::startsWith(trimmed, "#include ")) {
                result += processInclude(trimmed.substr(9), lineNum) + "\n";
            } else {
                result += expandMacros(line) + "\n";
            }
        }
        return result;
    }

    void define(const std::string& name, const std::string& value = "1") {
        macros_[name] = value;
    }

    bool isDefined(const std::string& name) const {
        return macros_.count(name) > 0;
    }

private:
    ErrorHandler& err_;
    std::string filename_;
    std::map<std::string, std::string> macros_;

    void processDefine(const std::string& expr, int line) {
        auto parts = Utils::split(expr, ' ');
        if (parts.empty()) return;
        std::string name = parts[0];
        std::string value = parts.size() > 1 ? Utils::join(std::vector<std::string>(parts.begin()+1, parts.end()), " ") : "1";
        macros_[name] = value;
    }

    std::string processInclude(const std::string& path, int line) {
        std::string p = path;
        if (!p.empty() && (p.front() == '"' || p.front() == '<'))
            p = p.substr(1, p.size()-2);
        if (!Utils::fileExists(p)) {
            err_.warn("Include file not found: " + p, line, filename_);
            return "";
        }
        try {
            std::string content = Utils::readFile(p);
            return process(content, p);
        } catch (...) {
            err_.warn("Could not read include: " + p, line, filename_);
            return "";
        }
    }

    std::string expandMacros(const std::string& line) {
        std::string result = line;
        for (auto& [name, value] : macros_) {
            result = Utils::replaceAll(result, name, value);
        }
        return result;
    }
};

#endif // PREPROCESSOR_HPP
