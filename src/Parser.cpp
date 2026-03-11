/**
 * @file Parser.cpp
 * @brief Implementation of Parser class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-09
 */

#include "Parser.h"
#include "Tokenizer.h"
#include <stdexcept>
#include <string>

EdgeNode::EdgeNode(std::vector<Edge> ed) : edges(std::move(ed)) {}
EdgeNode::EdgeNode(std::vector<Edge> ed, bool grp)
    : edges(std::move(ed)), grouped(grp) {}

Program Parser::parse() {
    auto graphs = parseGraphDecl();
    return Program(std::move(graphs));
}

GraphDeclaration Parser::parseGraphDecl() {
    consumeToken(TokenTag::GRAPH, "Expected the 'graph' keyword");
    std::string name = consumeString("Expected a name for the graph");
    consumeToken(TokenTag::LBRACE, "Graph declaration starts with '{'");
    auto body = parseGraphBody();
    consumeToken(TokenTag::RBRACE, "Graph declaration ends with '}'");
    return {name, body};
}

GraphBody Parser::parseGraphBody() {
    std::vector<NodeDeclaration> nodeDecls;
    std::vector<EdgeNode> edgeStmts;

    while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
        if (matchesType(TokenTag::NODE)) {
            auto node = parseNodeDecl();
            nodeDecls.emplace_back(node);
        } else {
            auto edge = parseEdgeStmt();
            edgeStmts.emplace_back(edge);
        }
    }
    return {nodeDecls, edgeStmts};
}

NodeDeclaration Parser::parseNodeDecl() {
    consumeToken(TokenTag::NODE, "Node declaration starts with 'node' keyword");
    std::string name = consumeIdentifier("A node declaration needs a name");
    auto t = peek();
    if (t.getType() == TokenTag::STRINGLITERAL) {
        auto s = t.type.as_string();
        advance();
        return {name, NodeBody::simple(s)};
    } else if (t.getType() == TokenTag::LBRACE) {
        advance();
        auto b = parseExpNode();
        // advance(); // ending RBrace
        return {name, b};
    }
    throw std::runtime_error(
        "SyntaxErr: Node declaration doesnt match the required syntax");
}

NodeBody Parser::parseExpNode() {
    std::vector<NodeField> ndFields;
    while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
        // advance();
        if (peek().getType() == TokenTag::TITLE) {
            advance(); // title itself
            consumeToken(TokenTag::COLON, "Need a colon after title");
            auto title = consumeString("Titles need a string value defined");
            auto nd = NodeField::title(title);
            ndFields.emplace_back(nd);
        } else if (peek().getType() == TokenTag::STYlE) {
            advance(); // style keyword
            StyleFields sf;
            consumeToken(TokenTag::LBRACE,
                         "Need a starting brace after style keyword");
            while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
                auto currentToken = peek();
                auto current = currentToken.getType();

                if (current == TokenTag::FILL) {
                    advance(); // fill
                    consumeToken(TokenTag::COLON,
                                 "Need a colon after fill keyword");
                    // TODO: will this drop an exception in case its not a bool?
                    auto val = advance().type.as_bool();
                    auto fl = Style::filling(val);
                    sf.emplace_back(fl);
                } else if (current == TokenTag::COLOR) {
                    advance(); // color
                    consumeToken(TokenTag::COLON,
                                 "Need a colon after color keyword");
                    // TODO: will this drop an exception in case its not a
                    // string?
                    auto val = advance().type.as_string();
                    auto vl = Style::color(val);
                    sf.emplace_back(vl);
                } else if (current == TokenTag::SHAPE) {
                    advance(); // shape
                    consumeToken(TokenTag::COLON,
                                 "Need a colon after shape keyword");
                    // TODO: will this drop an exception in case its not a
                    // string?
                    if (peek().getType() == TokenTag::CIRCLE) {
                        advance(); // circle
                        auto shp = Style::shape(Shapes::CIRCLE);
                        sf.emplace_back(shp);
                    } else if (peek().getType() == TokenTag::SQUARE) {
                        advance();
                        auto shp = Style::shape(Shapes::SQUARE);
                        sf.emplace_back(shp);
                    } else {
                        throw std::runtime_error("Shape style field support "
                                                 "either 'Circle' or 'Square'");
                    }
                } else {
                    printToken(currentToken);
                    throw std::runtime_error("Style fields support "
                                             "'Shape', 'Color' and'Filling'");
                }
            }
            consumeToken(TokenTag::RBRACE,
                         "A Node Style block should end with a '}'");
            ndFields.emplace_back(NodeField::style(sf));
        }
    }
    consumeToken(TokenTag::RBRACE,
                 "A Node declaration block should end with a '}'");
    return NodeBody::expanded(ndFields);
}

// start -> browse -> cart
// As:
// Type: Identifier(start), line: 36, column: 5
// Type: Arrow, line: 36, column: 11
// Type: Identifier(browse), line: 36, column: 14
// Type: Arrow, line: 36, column: 21
// Type: Identifier(cart), line: 36, column: 24
//
// cart -> "proceed" -> checkout -> "confirm" -> review
// As:
// Type: Identifier(cart), line: 39, column: 5
// Type: Arrow, line: 39, column: 10
// Type: StringLiteral("proceed), line: 39, column: 13
// Type: Arrow, line: 39, column: 23
// Type: Identifier(checkout), line: 39, column: 26
// Type: Arrow, line: 39, column: 35
// Type: StringLiteral("confirm), line: 39, column: 38
// Type: Arrow, line: 39, column: 48
// Type: Identifier(review), line: 39, column: 51
// TODO: chaining and grouping will be added here
EdgeNode Parser::parseEdgeStmt() {
    // two cases, one which is unlabeled
    // and one where the edge has a label
    // we have a node with its name used in edgenode
    // this node will then branch into edges which is saved inside edges as a
    // vector
    std::vector<Edge> edges;

    auto identifier =
        consumeIdentifier("Edge statements start with a node identifier");
    while (!isAtEnd() && matchesType(TokenTag::ARROW)) {
        advance(); // arrow
        // TODO: edgenode needs to either become a tree or change to
        // a simple vector . the second options results in combination
        // of edgenode and edge to have one structure containing edges which
        // have a name(ident) and an optional label. this structure will
        // containing a vector of these edges we could use the already defined
        // graphdeclaration edge vector
        if (matchesType(TokenTag::STRINGLITERAL)) {
            // labeled edge: ident -> "label" -> next
            auto label = peek().type.as_string();
            advance(); // consume the literal
            consumeToken(TokenTag::ARROW,
                         "Expected an arrow after an identifier");
            auto next = consumeIdentifier(
                "Expected a node identifier after edge arrow");
            edges.emplace_back(identifier, label);
            identifier = next;

        } else if (matchesType(TokenTag::IDENTIFIER)) {
            // bare edge: ident -> next
            edges.emplace_back(identifier);
            identifier = consumeIdentifier("Expected node identifier");
        } else {
            std::cerr << "Expected identifier or string after '->'\n";
            break;
        }
    }
    edges.emplace_back(identifier);
    return EdgeNode(edges, false);
}

void printProgram(Program &program) {
    std::cout << "Program:{\n";
    std::cout << "Graph:\n";
    printGraph(program.getGraphDecls().getBody());
    std::cout << "}";
}

void printGraph(const GraphBody &body) {
    std::cout << "Body:\n" << "Nodes:\n";
    for (auto &n : body.getNodes()) {
        std::cout << n.getName() << "\n";
    }
    std::cout << "Edges:\n";
    for (auto &e : body.getEdges()) {
        std::cout << "EdgeNode:";
        for (auto &n : e.edges) {
            std::cout << n.name << " ";
        }
        std::cout << std::endl;
    }
}
