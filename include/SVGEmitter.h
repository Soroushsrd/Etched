/**
 * @file SVGEmitter.h
 * @brief Declaration of SVGEmitter class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-18
 */

#pragma once
#include "Layout.h"
#include "SVGBuilder.h"

class SVGEmitter {
  public:
    SVGEmitter() = default;
    std::string emit(const LayoutResult &result);

  private:
    SVGBuilder builder;

    void emitNode(const LayoutNode &node);
    void emitEdge(const LayoutEdge &edge, const LayoutResult &result);
};
