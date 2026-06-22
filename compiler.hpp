// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "ast.hpp"
#include "code_generator.hpp"
#include "optimizer.hpp"
#include "semantic_analyzer.hpp"
#include "type_checker.hpp"
#include "error_handler.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <string>
#include <memory>

class Compiler {
public:
    Compiler(const Config& cfg, ErrorHandler& err)
        : cfg_(cfg), err_(err) {}

    bool compile(ASTNodePtr ast, const std::string& outputPath) {
        if (!ast) return false;

        // Semantic analysis
        SemanticAnalyzer sem(err_);
        sem.analyze(ast);
        if (err_.hasErrors()) return false;

        // Type checking
        TypeChecker tc(err_);
        tc.check(ast);
        if (err_.hasErrors()) return false;

        // Code generation
        TargetLang target = resolveTarget();
        CodeGenerator gen(target, err_);
        std::string code = gen.generate(ast);
        if (err_.hasErrors()) return false;

        // Write output
        std::string ext = targetExtension(target);
        std::string outFile = outputPath.empty() ? "out" + ext : outputPath;
        if (!Utils::writeFile(outFile, code)) {
            err_.report(ErrorCode::FILE_ERROR, "Cannot write output: " + outFile);
            return false;
        }

        if (cfg_.verboseMode)
            std::cout << "Compiled to: " << outFile << std::endl;

        return true;
    }

    std::string transpile(ASTNodePtr ast) {
        if (!ast) return "";
        TargetLang target = resolveTarget();
        CodeGenerator gen(target, err_);
        return gen.generate(ast);
    }

private:
    const Config& cfg_;
    ErrorHandler& err_;

    TargetLang resolveTarget() const {
        if (cfg_.target == "cpp")    return TargetLang::CPP;
        if (cfg_.target == "python") return TargetLang::PYTHON;
        if (cfg_.target == "csharp") return TargetLang::CSHARP;
        if (cfg_.target == "js")     return TargetLang::JS;
        return TargetLang::CPP;
    }

    std::string targetExtension(TargetLang t) const {
        switch (t) {
            case TargetLang::CPP:    return ".cpp";
            case TargetLang::PYTHON: return ".py";
            case TargetLang::CSHARP: return ".cs";
            case TargetLang::JS:     return ".js";
            default:                 return ".cpp";
        }
    }
};

#endif // COMPILER_HPP
