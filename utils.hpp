// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>

namespace Utils {

inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim)) result.push_back(token);
    return result;
}

inline std::string join(const std::vector<std::string>& v, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) result += sep;
        result += v[i];
    }
    return result;
}

inline bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

inline bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

inline std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

inline std::string repeat(const std::string& s, int n) {
    std::string result;
    for (int i = 0; i < n; i++) result += s;
    return result;
}

inline std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

inline bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return true;
}

inline bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

inline std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

inline std::string indent(const std::string& code, int spaces = 4) {
    std::string pad(spaces, ' ');
    auto lines = split(code, '\n');
    for (auto& l : lines) l = pad + l;
    return join(lines, "\n");
}

} // namespace Utils

#endif // UTILS_HPP
