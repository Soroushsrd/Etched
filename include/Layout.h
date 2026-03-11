/**
 * @file Layout.h
 * @brief Grid-based graph layout using topological sort (Kahn's algo)
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-12
 */

#pragma once

#include "Parser.h"
#include <string>
#include <unordered_map>
#include <vector>
struct LayoutNode {
    std::string id;
    std::string title;

    // grid coordiates (assigned by topo sort)
    int col = 0;
    int row = 0;

    // pixel coordiate (computed from grid coords + cell size)
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    bool isCircle = false;
};

struct LayoutEdge {
    std::string from;
    std::string to;
    // empty if unlabeled
    std::string label;
};

struct LayoutResult {
    std::vector<LayoutNode> nodes;
    std::vector<LayoutEdge> edges;
    double totalWidth = 0.0;
    double totalHeight = 0.0;
};

class Layout {
  public:
    // run the full layout pipeline on a parsed GraphBody
    // returns positioned nodes and routed edges ready for svg emission
    LayoutResult compute(const GraphBody &body);

  private:
    // extract a flat adjacency list + node info from AST
    void buildGraph(const GraphBody &body);
    // Kahn's algo for topo sort
    std::vector<std::string> topoSort();
    // assign grid positions to pxel coordinates
    void assignGridPositions(const std::vector<std::string> &order);
    // convert grid position to pixel coordinates
    void computePixelCoords();

    double measureTxtWidth(const std::string &txt) const;
    std::string resolveTitle(const std::string &id) const;

    // configs
    // horizontal spacing between cols
    static constexpr double cellW = 220.0;
    // vertical spacing between rows
    static constexpr double cellH = 120.0;
    // left/ top margin
    static constexpr double padX = 60.0;
    static constexpr double padY = 60.0;
    // default node box height
    static constexpr double nodeH = 50.0;
    // char width for sizing
    static constexpr double charW = 8.5;
    // horizontal padding inside node
    static constexpr double textPad = 220.0;

    // intermediate state
    std::unordered_map<std::string, const NodeDeclaration *> declMap;
    std::unordered_map<std::string, LayoutNode> nodeMap;
    std::vector<LayoutEdge> edges;

    // adjacency info for topo sort
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> inDegree;
    // all node ids in insertion order
    std::vector<std::string> allIDS;
};
