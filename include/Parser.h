/**
 * @file Parser.h
 * @brief Declaration of Parser class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-09
 */

#pragma once

#include "Tokenizer.h"
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct Edge;
/// example:
///   auth
///   ├── "success" -> ok
///   │   └── "load" -> dashboard
///   └── "invalid" -> fail
///       ├── "path1" -> output1
///       └── "path2" -> output2
struct EdgeNode {
    // refs to a declared node id
    // std::string name;
    // outgoing edges from this node. empty for leaf nodes
    std::vector<Edge> edges;
    // is this node grouped in source? for && fork resolution
    bool grouped = false;

    EdgeNode(std::vector<Edge> ed);
    EdgeNode(std::vector<Edge> ed, bool grp);
};

/// a singled directed edge. has an optional label and a target node
/// Bare:   auth -> ok           => Edge { label: None, target: EdgeNode { name:
/// "ok", ... } } Labeled: auth -> "success" -> ok => Edge { label:
/// Some("success"), target: EdgeNode { name: "ok", ... } }
struct Edge {
    std::string name;
    std::optional<std::string> label;
    Edge(const std::string &name, std::optional<std::string> lbl = std::nullopt)
        : name(name), label(lbl) {}
    // EdgeNode target;

    // Edge(EdgeNode trgt, std::optional<std::string> lbl = std::nullopt)
    //     : label(lbl), target(trgt) {}
};

enum class Shapes {
    /// circles have to have their diameter defined
    /// TODO: for now we define these with default values
    CIRCLE,
    /// squares need their dimentions defined (x,y)
    /// TODO: for now we define these with default values
    SQUARE,
};

using StyleValue = std::variant<std::string, bool, Shapes>;

class Style {
  public:
    enum class Type {
        FILLING,
        COLOR,
        SHAPE,
    } type;

  private:
    StyleValue value;

    Style(Type t, StyleValue v) : type(t), value(std::move(v)) {}

  public:
    static Style filling(bool b) { return {Type::FILLING, b}; }
    static Style color(std::string s) { return {Type::COLOR, std::move(s)}; }
    static Style shape(Shapes s) { return {Type::SHAPE, s}; }

    Type getType() const { return type; }
    const std::string &as_string() const {
        return std::get<std::string>(value);
    }
    bool as_bool() const { return std::get<bool>(value); }
    Shapes as_shape() const { return std::get<Shapes>(value); }
};

using StyleFields = std::vector<Style>;
using NodeValues = std::variant<std::string, StyleFields>;
struct NodeField {
  public:
    enum class NodeType {
        TITLE,
        STYLE,
    } type;

  private:
    NodeValues value;
    NodeField(NodeType nt, NodeValues nv) : type(nt), value(std::move(nv)) {}

  public:
    static NodeField title(std::string str) {
        return {NodeType::TITLE, std::move(str)};
    }
    static NodeField style(StyleFields sf) {
        return {NodeType::STYLE, std::move(sf)};
    }

    NodeType getType() const { return type; }
    const std::string &as_string() const {
        return std::get<std::string>(value);
    }
    const StyleFields &as_style() const { return std::get<StyleFields>(value); }
};

using NodeBodyValue = std::variant<std::string, std::vector<NodeField>>;
class NodeBody {
  public:
    enum class Type {
        SIMPLE,
        EXPANDED,
    } type;

  private:
    NodeBodyValue value;
    NodeBody(Type tt, NodeBodyValue val) : type(tt), value(std::move(val)) {}

  public:
    NodeBody() = default;
    static NodeBody simple(std::string str) {
        return {Type::SIMPLE, std::move(str)};
    }
    static NodeBody expanded(std::vector<NodeField> fields) {
        return {Type::EXPANDED, std::move(fields)};
    }

    Type getType() const { return type; }
    const std::string &as_string() const {
        return std::get<std::string>(value);
    }
    const std::vector<NodeField> &as_fields() const {
        return std::get<std::vector<NodeField>>(value);
    }
};

class NodeDeclaration {
  private:
    std::string name;
    NodeBody body;

  public:
    NodeDeclaration(std::string n, NodeBody b)
        : name(std::move(n)), body(std::move(b)) {}

    const std::string &getName() const { return name; }
    const NodeBody &getBody() const { return body; }
};

class GraphBody {
  private:
    std::vector<NodeDeclaration> nodeDeclarations;
    std::vector<EdgeNode> edgeStatements;

  public:
    GraphBody(std::vector<NodeDeclaration> decls, std::vector<EdgeNode> stmts)
        : nodeDeclarations(std::move(decls)), edgeStatements(std::move(stmts)) {
    }

    const std::vector<NodeDeclaration> &getNodes() const {
        return nodeDeclarations;
    }
    const std::vector<EdgeNode> &getEdges() const { return edgeStatements; }
};

class GraphDeclaration {
  private:
    std::string name;
    GraphBody body;

  public:
    GraphDeclaration(std::string n, GraphBody b)
        : name(std::move(n)), body(std::move(b)) {}
    const std::string &getName() const { return name; }
    const GraphBody &getBody() const { return body; }
};

class Program {
  private:
    GraphDeclaration graphDecls;

  public:
    Program(GraphDeclaration g) : graphDecls(std::move(g)) {}
    const GraphDeclaration &getGraphDecls() const { return graphDecls; }
};

class Parser {
  public:
    Parser(std::vector<Token> tkn) : tokens(std::move(tkn)) {}
    Program parse();

  private:
    GraphDeclaration parseGraphDecl();
    GraphBody parseGraphBody();
    NodeDeclaration parseNodeDecl();
    NodeBody parseExpNode();
    EdgeNode parseEdgeStmt();
    EdgeNode parseEdgeGrp();
    EdgeNode parseEdgeChain();

    const Token &peek() const { return tokens.at(current); }
    const Token &advance() {
        if (!isAtEnd()) {
            current++;
        }
        return prev();
    }
    const Token &prev() const {
        if (current > 0) {
            return tokens.at(current - 1);
        }
        return tokens.at(current);
    }
    bool isAtEnd() const { return peek().getType() == TokenTag::END; };
    bool matchesType(TokenTag tt) { return peek().getType() == tt; }
    void consumeToken(TokenTag tt, const std::string &errMsg = "") {
        if (matchesType(tt)) {
            advance();
        } else {
            std::cerr << "Missing token: " << errMsg << std::endl;
        }
    }
    std::string consumeString(const std::string &errMsg = "") {
        if (auto str = peek(); str.getType() == TokenTag::STRINGLITERAL) {
            advance();
            return str.type.as_string();
        }
        std::cerr << "SyntaxErr " << errMsg << std::endl;
        return "";
    }
    std::string consumeIdentifier(const std::string &errMsg = "") {
        if (auto str = peek(); str.getType() == TokenTag::IDENTIFIER) {
            advance();
            return str.type.as_string();
        }
        std::cerr << "SyntaxErr " << errMsg << std::endl;
        return "";
    }

  private:
    // vector of tokens passed from tokenizer
    std::vector<Token> tokens;
    // index of the current token in token vector
    size_t current = 0;
};

void printGraph(const GraphBody &body);
void printProgram(Program &program);
