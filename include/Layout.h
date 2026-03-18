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

// Represents a single node after layout has been computed.
// It holds the node's id (matching what the parser gave it),
// a readable title (node auth "login": id->auth and title->Login),
// grid coordinates (col,row), pixl coordinates (x, y) dimensions (width,
// height), and a shape aware flag
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

// flat, render friendly edge
// Since parser's structure (EdgeNode, Edge) is tree shaped and harder to
// iterate, flattening it using this struct allows easier iteration in the next
// pass
struct LayoutEdge {
    std::string from;
    std::string to;
    // empty if unlabeled
    std::string label;
};

// Output contract and everything the renderer needs.
// Nodes wit hcomputed positions, flat edges, and total canvas dimensions are
// inside this struct which allows sizing the SVG viewBox
struct LayoutResult {
    std::vector<LayoutNode> nodes;
    std::vector<LayoutEdge> edges;
    double totalWidth = 0.0;
    double totalHeight = 0.0;

    void print() {
        std::cout << "=== Layout Result ===\n";
        std::cout << "Canvas: " << totalWidth << " x " << totalHeight << "\n\n";

        std::cout << "Nodes:\n";
        for (const auto &n : nodes) {
            std::cout << "  " << n.id << " [" << n.title << "]"
                      << "  grid=(" << n.col << "," << n.row << ")"
                      << "  pos=(" << n.x << "," << n.y << ")"
                      << "  size=" << n.width << "x" << n.height
                      << (n.isCircle ? "  CIRCLE" : "") << "\n";
        }

        std::cout << "\nEdges:\n";
        for (const auto &e : edges) {
            std::cout << "  " << e.from << " -> " << e.to;
            if (!e.label.empty())
                std::cout << "  [" << e.label << "]";
            std::cout << "\n";
        }
    }
};

class Layout {
  public:
    // run the full layout pipeline on a parsed GraphBody
    // returns positioned nodes and routed edges ready for svg emission
    LayoutResult compute(const GraphBody &body);

  private:
    // extract a flat adjacency list + node info from AST
    // Its the first pass! builds declMap from node declarations, registers
    // every declared node into nodeMap (resolves titles and checks shapes)
    // then walks every EdgeNode in the edge stmts to populate adk,inDegree
    // and edges
    void buildGraph(const GraphBody &body);
    // Kahn's algo for topo sort
    // runs Kahns algo on adj and inDegree. Seeds the BFS queue wit hzero
    // indegree nodes then repeatedly pops a node, appends it to order,
    // decrements the indegree of its neighbours
    // read the **docs/Topological Ordering.org** file
    std::vector<std::string> topoSort();
    // assign grid positions to pxel coordinates
    // takes the topo order and assigns col/row to each node using longest
    // path layering. For each node in order, it scan edges to find all
    // preds and takes max(predecessor_col)+1 as its column meaning the node
    // that depends on two preds gets pushed right of both. within each
    // column a colRowCount map tracks the next available row so multiple
    // nodes in the same column stack vertically
    void assignGridPositions(const std::vector<std::string> &order);
    // convert grid position to pixel coordinates
    void computePixelCoords();

    // estimates how wide a node box has to be based on its
    // title's character count.
    // TODO: right now it uses charCount * 8.5 + 220. we need
    // to use font metrics for real text measurement
    double measureTxtWidth(const std::string &txt) const;

    // helper method
    // Given a node id, checks if declMap has it.  if not, the id is the
    // tile for that node
    std::string resolveTitle(const std::string &id) const;

    // ------------------
    // | Configurations |
    // ------------------

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

    // ----------------------
    // | Intermediate State |
    // ----------------------

    // id -> NodeDeclaration*
    // a lookup table built from parsed node declarations. used for
    // resolving titles and checking for circle shapes
    std::unordered_map<std::string, const NodeDeclaration *> declMap;
    // id->LayoutNode
    // a working state for every node seen so far wether declared or refd in
    // an edge. Nodes that appear in edges but have no node declaration are
    // still valid they get their id as their title
    std::unordered_map<std::string, LayoutNode> nodeMap;
    // a flat list that gets built during buildGraph() method.
    std::vector<LayoutEdge> edges;

    // ---------------------------------------
    // | Adjacency Info For Topological Sort |
    // ------------------ --------------------

    // adj[src] lists all nodes src points to!
    std::unordered_map<std::string, std::vector<std::string>> adj;
    // inDegree[id] counts how many nodes point into id
    std::unordered_map<std::string, int> inDegree;
    // all node ids in insertion order
    // allIDS preserves insertion order for all node ids. This matters
    // because std::unordered_map doesn't give me a stable iteration order.
    // when seeding Kahn's queue, we want a deterministic traversal order,
    // not hash-bucket order.
    std::vector<std::string> allIDS;
};
