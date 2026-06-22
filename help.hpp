// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef HELP_HPP
#define HELP_HPP

#include <iostream>
#include "version.hpp"

inline void printHelp() {
    std::cout << R"(
  ___        _    _
 / _ \      | |  | |
/ /_\ \ ___ | |  | |
|  _  |/ _ \| |/\| |
| | | |  __/\  /\  /
\_| |_/\___| \/  \/

)" << getVersionString() << R"(

USAGE:
  app [options] <file.app>

OPTIONS:
  -h, --help            Show this help message
  -v, --version         Show version information
  -o, --output <file>   Set output file name
  -t, --target <lang>   Set compilation target (native, cpp, python, csharp, js)
  -O<level>             Set optimization level (0-3)
  -g, --debug           Enable debug mode
  --verbose             Enable verbose output
  --show-ast            Print the AST after parsing
  --show-tokens         Print tokens after tokenization
  --repl                Start the interactive REPL
  --run                 Run after compiling

TARGETS:
  native    Compile to native machine code (default)
  cpp       Transpile to C++
  python    Transpile to Python
  csharp    Transpile to C#
  js        Transpile to JavaScript

EXAMPLES:
  app hello.app                   Run hello.app
  app -t cpp hello.app            Transpile to C++
  app -O2 -o myapp hello.app      Compile with optimizations
  app --repl                      Start interactive REPL

DOCUMENTATION:
  https://aplusplus-lang.dev

)" << std::endl;
}

inline void printVersion() {
    std::cout << getFullVersionString() << std::endl;
}

#endif // HELP_HPP
