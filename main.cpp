/**
 * @file main.cpp
 * @brief Main entry point
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#include "Layout.h"
#include "Result.h"
#include "SVGEmitter.h"
#include <fstream>
#include <iostream>
#include <sstream>

void run_file(std::ifstream &file) {
    std::stringstream ss;
    ss << file.rdbuf();
    std::string sourceString = ss.str();

    Tokenizer tokenizer(sourceString);
    auto tokens = tokenizer.Tokenize();
    if (tokens.isErr()) {
        return;
    }
    Parser parser(tokens.value());
    auto pr = parser.parse();

    Layout layout;
    auto result = layout.compute(pr.getGraphDecls().getBody());
    result.print();

    SVGEmitter emitter;
    auto svg = emitter.emit(result);

    std::ofstream output("outputtest.svg");
    if (!output) {
        std::cerr << "Could not write output.svg\n";
        return;
    }
    output << svg;
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
