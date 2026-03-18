/**
 * @file SVGBuilder.h
 * @brief Declaration of SVGBuilder class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-18
 */

#pragma once

#include <string>
#include <vector>
class SVGBuilder {
  public:
    SVGBuilder() = default;

    std::string build(double totalWidth, double totalHeight);

    void addRect(double x, double y, double w, double h,
                 const std::string &fill = "white",
                 const std::string &stroke = "black", double strokeWidth = 1.5,
                 double rx = 4.0);
    void addCircle(double cx, double cy, double r,
                   const std::string &fill = "white",
                   const std::string &stroke = "black",
                   double strokeWidth = 1.5);
    void addLine(double x1, double y1, double x2, double y2,
                 const std::string &stroke = "black", double strokeWidth = 1.5,
                 bool arrow = false);
    void addText(double x, double y, const std::string &content,
                 const std::string &anchor = "middle", double fontSize = 13.0);
    // call once before any addLine with arrow=true!
    void addArrowHead();

  private:
    // <def> blocks like arrowhead marker
    std::vector<std::string> defs;
    // other elements in document order
    std::vector<std::string> elements;
    bool arrowDefAdded = false;

    static std::string fmt(double d);
    static std::string escape(const std::string &s);
};
