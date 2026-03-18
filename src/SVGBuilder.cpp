/**
 * @file SVGBuilder.cpp
 * @brief Implementation of SVGBuilder class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-18
 */

#include "SVGBuilder.h"
#include <iomanip>
#include <ios>
#include <sstream>

std::string SVGBuilder::fmt(double d) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << d;
    return os.str();
}

std::string SVGBuilder::escape(const std::string &s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '"':
            result += "&quote;";
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}

void SVGBuilder::addRect(double x, double y, double w, double h,
                         const std::string &fill, const std::string &stroke,
                         double strokeWidth, double rx) {

    std::string el = "<rect"
                     " x=\"" +
                     fmt(x) +
                     "\""
                     " y=\"" +
                     fmt(y) +
                     "\""
                     " width=\"" +
                     fmt(w) +
                     "\""
                     " height=\"" +
                     fmt(h) +
                     "\""
                     " rx=\"" +
                     fmt(rx) +
                     "\""
                     " fill=\"" +
                     fill +
                     "\""
                     " stroke=\"" +
                     stroke +
                     "\""
                     " stroke-width=\"" +
                     fmt(strokeWidth) +
                     "\""
                     " />";
    elements.push_back(el);
}

void SVGBuilder::addCircle(double cx, double cy, double r,
                           const std::string &fill, const std::string &stroke,
                           double strokeWidth) {
    std::string el = "<circle"
                     " cx=\"" +
                     fmt(cx) +
                     "\""
                     " cy=\"" +
                     fmt(cy) +
                     "\""
                     " r=\"" +
                     fmt(r) +
                     "\""
                     " fill=\"" +
                     fill +
                     "\""
                     " stroke=\"" +
                     stroke +
                     "\""
                     " stroke-width=\"" +
                     fmt(strokeWidth) +
                     "\""
                     " />";
    elements.push_back(el);
}

void SVGBuilder::addLine(double x1, double y1, double x2, double y2,
                         const std::string &stroke, double strokeWidth,
                         bool arrow) {
    std::string el = "<line"
                     " x1=\"" +
                     fmt(x1) +
                     "\""
                     " y1=\"" +
                     fmt(y1) +
                     "\""
                     " x2=\"" +
                     fmt(x2) +
                     "\""
                     " y2=\"" +
                     fmt(y2) +
                     "\""
                     " stroke=\"" +
                     stroke +
                     "\""
                     " stroke-width=\"" +
                     fmt(strokeWidth) + "\"";

    if (arrow) {
        el += " marker-end=\"url(#arrowhead)\"";
    }

    el += "/>";
    elements.push_back(el);
}

void SVGBuilder::addText(double x, double y, const std::string &content,
                         const std::string &anchor, double fontSize) {
    std::string el = "<text"
                     " x=\"" +
                     fmt(x) +
                     "\""
                     " y=\"" +
                     fmt(y) +
                     "\""
                     " text-anchor=\"" +
                     anchor +
                     "\""
                     " dominant-baseline=\"middle\""
                     " font-size=\"" +
                     fmt(fontSize) +
                     "\""
                     " font-family=\"sans-serif\""
                     ">" +
                     escape(content) + "</text>";
    elements.push_back(el);
}

void SVGBuilder::addArrowHead() {
    if (arrowDefAdded)
        return;
    // a simple filled triangle, 10px long, 7px wide
    // refX is set to 10 (tip of the triangle) so the line ends at the tip
    // not at the base
    std::string marker =
        "<marker"
        " id=\"arrowhead\""
        " markerWidth=\"10\""
        " markerHeight=\"7\""
        " refX=\"10\""
        " refY=\"3.5\""
        " orient=\"auto\""
        ">"
        "<polygon points=\"0 0, 10 3.5, 0 7\" fill=\"black\" />"
        "</marker>";
    defs.push_back(marker);
    arrowDefAdded = true;
}

std::string SVGBuilder::build(double totalWidth, double totalHeight) {
    std::string out;
    out += "<svg"
           " xmlns=\"http://www.w3.org/2000/svg\""
           " viewBox=\"0 0 " +
           fmt(totalWidth) + " " + fmt(totalHeight) +
           "\""
           " width=\"" +
           fmt(totalWidth) +
           "\""
           " height=\"" +
           fmt(totalHeight) +
           "\""
           ">\n";

    if (!defs.empty()) {
        out += "  <defs>\n";
        for (const auto &d : defs) {
            out += "    " + d + "\n";
        }
        out += "  </defs>\n";
    }
    for (const auto &el : elements) {
        out += "  " + el + "\n";
    }

    out += "</svg>\n";
    return out;
}
