// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "tokenizer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "compiler.hpp"
#include "optimizer.hpp"
#include "debugger.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "version.hpp"
#include "help.hpp"
#include "runtime.hpp"
#include "standard_library.hpp"
#include "preprocessor.hpp"

static Config parseArgs(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")    { printHelp(); exit(0); }
        if (arg == "-v" || arg == "--version") { printVersion(); exit(0); }
        if (arg == "--repl")   { cfg.replMode = true; continue; }
        if (arg == "--verbose"){ cfg.verboseMode = true; continue; }
        if (arg == "-g" || arg == "--debug") { cfg.debugMode = true; continue; }
        if (arg == "--show-ast")    { cfg.showAST = true; continue; }
        if (arg == "--show-tokens") { cfg.showTokens = true; continue; }
        if (arg == "--show-ir")     { cfg.showIR = true; continue; }
        if (arg == "--run")         { cfg.runAfterBuild = true; continue; }
        if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) cfg.outputFile = argv[++i];
            continue;
        }
        if (arg == "-t" || arg == "--target") {
            if (i+1 < argc) cfg.target = argv[++i];
            continue;
        }
        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == 'O') {
            cfg.optimizationLevel = (arg.size() > 2) ? (arg[2] - '0') : 1;
            continue;
        }
        if (arg[0] != '-') {
            cfg.sourceFile = arg;
            continue;
        }
        std::cerr << "Unknown option: " << arg << "\n";
    }
    return cfg;
}

static void runREPL(Config& cfg) {
    ErrorHandler err;
    Runtime rt(cfg, err);

    std::cout << getVersionString() << " REPL\n";
    std::cout << "Type 'exit' or Ctrl+D to quit. Type 'help' for help.\n\n";

    std::string line;
    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line == "help") { printHelp(); continue; }
        if (line.empty()) continue;

        try {
            Preprocessor pp(err);
            std::string processed = pp.process(line, "<repl>");
            Tokenizer tok(processed, "<repl>");
            auto tokens = tok.tokenize();
            Parser parser(tokens, err);
            auto ast = parser.parse();
            if (cfg.showAST) {
                Debugger dbg(err);
                dbg.printAST(ast);
            }
            rt.run(ast);
        } catch (const std::exception& e) {
            std::cerr << "\033[31mError\033[0m: " << e.what() << "\n";
        }
        err.clear();
    }
    std::cout << "\nGoodbye!\n";
}

static int runFile(Config& cfg) {
    ErrorHandler err;

    if (!Utils::fileExists(cfg.sourceFile)) {
        printError("File not found: " + cfg.sourceFile);
        return 1;
    }

    // Read & preprocess
    std::string source;
    try { source = Utils::readFile(cfg.sourceFile); }
    catch (const std::exception& e) {
        printError(std::string("Cannot read file: ") + e.what());
        return 1;
    }

    Preprocessor pp(err);
    source = pp.process(source, cfg.sourceFile);
    if (err.hasErrors()) return 1;

    // Tokenize
    Tokenizer tok(source, cfg.sourceFile);
    std::vector<Token> tokens;
    try { tokens = tok.tokenize(); }
    catch (const std::exception& e) {
        printError(e.what());
        return 1;
    }

    if (cfg.showTokens) {
        for (auto& t : tokens) {
            std::cout << "[" << t.line << ":" << t.column << "] "
                      << t.value << "\n";
        }
    }

    // Parse
    Parser parser(tokens, err);
    ASTNodePtr ast;
    try { ast = parser.parse(); }
    catch (const std::exception& e) {
        printError(e.what());
        return 1;
    }
    if (err.hasErrors()) return 1;

    if (cfg.showAST) {
        Debugger dbg(err);
        dbg.printAST(ast);
    }

    // Optimize
    Optimizer opt(cfg.optimizationLevel, err);
    ast = opt.optimize(ast);

    // Compile or interpret
    if (cfg.target != "native" && !cfg.target.empty()) {
        Compiler comp(cfg, err);
        std::string out = cfg.outputFile;
        if (out == "a.out") {
            out = cfg.sourceFile.substr(0, cfg.sourceFile.rfind('.'));
            if (cfg.target == "cpp")    out += ".cpp";
            else if (cfg.target == "python") out += ".py";
            else if (cfg.target == "csharp") out += ".cs";
            else if (cfg.target == "js")     out += ".js";
        }
        if (!comp.compile(ast, out)) return 1;
        if (cfg.verboseMode) std::cout << "Written to: " << out << "\n";
    } else {
        Runtime rt(cfg, err);
        return rt.run(ast);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    Config cfg = parseArgs(argc, argv);

    if (cfg.replMode || (argc == 1)) {
        runREPL(cfg);
        return 0;
    }

    if (cfg.sourceFile.empty()) {
        printHelp();
        return 1;
    }

    return runFile(cfg);
}
