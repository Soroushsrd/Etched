/**
 * @file Layout.cpp
 * @brief Implementation of Grid Layout using Kahn's topo sort
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-12
 */

#include "Layout.h"
#include <algorithm>
#include <cstddef>
#include <queue>

std::string Layout::resolveTitle(const std::string &id) const {
    auto it = declMap.find(id);
    if (it == declMap.end()) {
        return id;
    }

    const auto &body = it->second->getBody();
    if (body.getType() == NodeBody::Type::SIMPLE) {
        return body.as_string();
    }

    // if expanded node
    for (const auto &field : body.as_fields()) {
        if (field.getType() == NodeField::NodeType::TITLE) {
            return field.as_string();
        }
    }
    return id;
}

double Layout::measureTxtWidth(const std::string &txt) const {
    return static_cast<double>(txt.size() * charW + textPad);
}

void Layout::buildGraph(const GraphBody &body) {
    // build id -> decl loopup
    for (const auto &nd : body.getNodes()) {
        declMap[nd.getName()] = &nd;
    }

    // registering all decl nodes
    for (const auto &nd : body.getNodes()) {
        const auto &id = nd.getName();
        if (nodeMap.find(id) == nodeMap.end()) {
            allIDS.push_back(id);
            inDegree[id] = 0;
            LayoutNode ln;
            ln.id = id;
            ln.title = resolveTitle(id);
            ln.width = measureTxtWidth(ln.title);
            ln.height = nodeH;

            // if circle
            if (declMap.count(id)) {
                const auto &nodeBody = declMap.at(id)->getBody();
                if (nodeBody.getType() == NodeBody::Type::EXPANDED) {
                    for (auto const &field : nodeBody.as_fields()) {
                        if (field.getType() == NodeField::NodeType::STYLE) {
                            for (const auto &s : field.as_style()) {
                                if (s.getType() == Style::Type::SHAPE &&
                                    s.as_shape() == Shapes::CIRCLE) {
                                    ln.isCircle = true;
                                }
                            }
                        }
                    }
                }
            }
            nodeMap[id] = ln;
        }
    }

    // extract edges from EdgeNode lists
    // each EdgeNode contains a sequential chain:
    //   edges[0] -> edges[1] -> edges[2] -> ...
    // if edges[i] has a label, that label belongs on the connection
    // FROM edges[i-1] TO edges[i] (not from i to i+1).
    //
    // the structure is:
    //   Edge{name="start"},  Edge{name="browse"},  Edge{name="cart"}
    // for:  start -> browse -> cart
    //
    // and for labeled:
    //   Edge{name="cart", label="proceed"}, Edge{name="checkout",
    //   label="confirm"}, Edge{name="review"}
    // for:  cart -> "proceed" -> checkout -> "confirm" -> review
    //
    // so: connection from edges[i] to edges[i+1], label is edges[i].label
    for (const auto &en : body.getEdges()) {
        for (size_t i = 0; i + 1 < en.edges.size(); i++) {
            const auto &src = en.edges[i].name;
            const auto &dst = en.edges[i + 1].name;
            std::string lbl =
                en.edges[i].label.has_value() ? en.edges[i].label.value() : "";

            // both nodes must exist in map
            // this handles nodes refd in edges but not declared with 'node'
            for (const auto &nid : {src, dst}) {
                if (nodeMap.find(nid) == nodeMap.end()) {
                    allIDS.push_back(nid);
                    inDegree[nid] = 0;
                    LayoutNode ln;
                    ln.id = nid;
                    ln.title = nid; // no declaration so we use id as title
                    ln.width = measureTxtWidth(ln.title);
                    ln.height = nodeH;
                    nodeMap[nid] = ln;
                }
            }
            adj[src].push_back(dst);
            inDegree[dst]++;
            edges.push_back({src, dst, lbl});
        }
    }
}

std::vector<std::string> Layout::topoSort() {
    std::vector<std::string> order;
    std::queue<std::string> q;

    // seed with zero inDegree nodes
    for (const auto &id : allIDS) {
        if (inDegree[id] == 0) {
            q.push(id);
        }
    }

    while (!q.empty()) {
        auto cur = q.front();
        q.pop();
        order.push_back(cur);

        if (adj.find(cur) != adj.end()) {
            for (const auto &neighbor : adj[cur]) {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0) {
                    q.push(neighbor);
                }
            }
        }
    }

    // nodes with no edges at all should still show up
    return order;
}

// simple strategy: assign columns by topo order,
// and use rows to spread out nodes that share a column.
//
// TODO: for now: each node gets its own column (left to right),
// all on row 0. this produces a horizontal chain.
// a smarter version would use longest-path layering for columns
// and stack siblings vertically.

// longest-path layering: col = max(col of predecessors) + 1
// this groups nodes into layers better than sequential assignment
void Layout::assignGridPositions(const std::vector<std::string> &order) {
    std::unordered_map<std::string, int> colAssign;
    for (const auto &id : order) {
        int maxPredcol = -1;
        // find predecessors
        for (const auto &e : edges) {
            if (e.to == id) {
                auto it = colAssign.find(e.from);
                if (it != colAssign.end()) {
                    maxPredcol = std::max(maxPredcol, it->second);
                }
            }
        }
        colAssign[id] = maxPredcol + 1;
    }
    // assigning orws. within each clmn, stacking nodes verticallt
    // col->next available row
    std::unordered_map<int, int> colRowcount;

    for (const auto &id : order) {
        int col = colAssign[id];
        int row = colRowcount[col]++;
        nodeMap[id].col = col;
        nodeMap[id].row = row;
    }
}

void Layout::computePixelCoords() {
    for (auto &[id, node] : nodeMap) {
        node.x = padX + node.col * cellW;
        node.y = padY + node.row * cellH;
    }
}

LayoutResult Layout::compute(const GraphBody &body) {
    buildGraph(body);

    auto order = topoSort();
    assignGridPositions(order);
    computePixelCoords();

    double maxX = 0;
    double maxY = 0;
    for (const auto &[id, n] : nodeMap) {
        maxX = std::max(maxX, n.x + n.width);
        maxY = std::max(maxY, n.y + n.height);
    }

    LayoutResult result;
    for (const auto &id : order) {
        result.nodes.push_back(nodeMap[id]);
    }

    result.edges = edges;
    result.totalWidth = maxX + padX;
    result.totalHeight = maxY + padY;

    return result;
}
