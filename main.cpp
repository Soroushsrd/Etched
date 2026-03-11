/**
 * @file main.cpp
 * @brief Main entry point
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#include "Parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

void run_file(std::ifstream &file) {
    std::stringstream ss;
    ss << file.rdbuf();
    std::string sourceString = ss.str();

    Tokenizer tokenizer(sourceString);
    tokenizer.Tokenize();
    for (auto &token : tokenizer.getTokens()) {
        printToken(token);
    }
    Parser parser(tokenizer.getTokens());
    auto pr = parser.parse();
    printProgram(pr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<filename>\n";
        return 1;
    }
    std::ifstream file(argv[1]);

    if (!file) {
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    run_file(file);

    return 0;
}
