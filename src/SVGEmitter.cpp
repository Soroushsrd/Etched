/**
 * @file SVGEmitter.cpp
 * @brief Implementation of SVGEmitter class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-18
 */

#include "SVGEmitter.h"
#include "Layout.h"

void SVGEmitter::emitNode(const LayoutNode &node) {
    if (node.isCircle) {
        double r = node.width / 2.0;
        double cx = node.x + r;
        double cy = node.y + r;
        builder.addCircle(cx, cy, r);
        builder.addText(cx, cy, node.title);
    } else {
        builder.addRect(node.x, node.y, node.width, node.height);
        double cx = node.x + node.width / 2.0;
        double cy = node.y + node.height / 2.0;
        builder.addText(cx, cy, node.title);
    }
}

void SVGEmitter::emitEdge(const LayoutEdge &edge, const LayoutResult &result) {
    const LayoutNode *from = nullptr;
    const LayoutNode *to = nullptr;

    for (const auto &n : result.nodes) {
        if (n.id == edge.from) {
            from = &n;
        }
        if (n.id == edge.to) {
            to = &n;
        }
    }
    if (!from || !to)
        return;

    // center of each node
    double x1 = from->x + from->width / 2.0;
    double y1 = from->y + from->height / 2.0;
    double x2 = to->x + to->width / 2.0;
    double y2 = to->y + to->height / 2.0;

    builder.addLine(x1, y1, x2, y2, "black", 1.5, true);
    if (!edge.label.empty()) {
        double mx = (x1 + x2) / 2.0;
        double my = (y1 + y2) / 2.0 - 12.0;
        builder.addText(mx, my, edge.label);
    }
}

std::string SVGEmitter::emit(const LayoutResult &result) {
    builder.addArrowHead();

    for (const auto &edge : result.edges) {
        emitEdge(edge, result);
    }
    for (const auto &node : result.nodes) {
        emitNode(node);
    }
    return builder.build(result.totalWidth, result.totalHeight);
}
