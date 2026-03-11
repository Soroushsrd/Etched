/**
 * @file main.cpp
 * @brief Main entry point
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#include "Layout.h"
#include <fstream>
#include <iostream>
#include <sstream>

void run_file(std::ifstream &file) {
    std::stringstream ss;
    ss << file.rdbuf();
    std::string sourceString = ss.str();

    Tokenizer tokenizer(sourceString);
    tokenizer.Tokenize();
    // for (auto &token : tokenizer.getTokens()) {
    //     printToken(token);
    // }
    Parser parser(tokenizer.getTokens());
    auto pr = parser.parse();
    // printProgram(pr);

    Layout layout;
    auto result = layout.compute(pr.getGraphDecls().getBody());
    std::cout << "=== Layout Result ===\n";
    std::cout << "Canvas: " << result.totalWidth << " x " << result.totalHeight
              << "\n\n";

    std::cout << "Nodes:\n";
    for (const auto &n : result.nodes) {
        std::cout << "  " << n.id << " [" << n.title << "]"
                  << "  grid=(" << n.col << "," << n.row << ")"
                  << "  pos=(" << n.x << "," << n.y << ")"
                  << "  size=" << n.width << "x" << n.height
                  << (n.isCircle ? "  CIRCLE" : "") << "\n";
    }

    std::cout << "\nEdges:\n";
    for (const auto &e : result.edges) {
        std::cout << "  " << e.from << " -> " << e.to;
        if (!e.label.empty())
            std::cout << "  [" << e.label << "]";
        std::cout << "\n";
    }
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
