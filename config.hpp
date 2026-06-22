// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>

struct Config {
    // Build modes
    bool debugMode     = false;
    bool optimizeMode  = false;
    bool verboseMode   = false;
    bool replMode      = false;

    // Target backend
    std::string target = "native"; // native | cpp | python | csharp | js

    // Paths
    std::string outputFile  = "a.out";
    std::string sourceFile  = "";
    std::string stdlib_path = "./stdlib";

    // Compiler settings
    int optimizationLevel = 0;  // 0-3
    bool emitWarnings     = true;
    bool treatWarningsAsErrors = false;
    bool showAST          = false;
    bool showTokens       = false;
    bool showIR           = false;

    // Execution settings
    bool runAfterBuild = false;

    // Extra user-defined settings
    std::map<std::string, std::string> extra;

    void set(const std::string& key, const std::string& value) {
        extra[key] = value;
    }

    std::string get(const std::string& key, const std::string& def = "") const {
        auto it = extra.find(key);
        return it != extra.end() ? it->second : def;
    }
};

#endif // CONFIG_HPP
