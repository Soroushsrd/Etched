/**
 * @file Parser.cpp
 * @brief Implementation of Parser class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-09
 */

#include "Parser.h"
#include "Result.h"
#include "Tokenizer.h"
#include <string>

EdgeNode::EdgeNode(std::vector<Edge> ed) : edges(std::move(ed)) {}
EdgeNode::EdgeNode(std::vector<Edge> ed, bool grp)
    : edges(std::move(ed)), grouped(grp) {}

Result<Program> Parser::parse() {
    auto graphsRes = parseGraphDecl();
    if (!graphsRes) {
        return Result<Program>::err(graphsRes.error());
    }
    return Result<Program>::ok(Program(std::move(graphsRes.value())));
}

Result<GraphDeclaration> Parser::parseGraphDecl() {
    TRY_VOID(consumeToken(TokenTag::GRAPH, "Expected the 'graph' keyword"));
    TRY(name, consumeString("Expected a name for the graph"));
    TRY_VOID(consumeToken(TokenTag::LBRACE, "Expected '{' after graph name"));
    TRY(body, parseGraphBody());
    TRY_VOID(consumeToken(TokenTag::RBRACE, "Graph declaration ends with '}'"));

    return Result<GraphDeclaration>::ok({name, body});
}

Result<GraphBody> Parser::parseGraphBody() {
    std::vector<NodeDeclaration> nodeDecls;
    std::vector<EdgeNode> edgeStmts;

    while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
        if (matchesType(TokenTag::NODE)) {
            TRY(nodeRes, parseNodeDecl());
            nodeDecls.emplace_back(nodeRes);
        } else {
            TRY(edgeRes, parseEdgeStmt());
            edgeStmts.emplace_back(edgeRes);
        }
    }
    return Result<GraphBody>::ok({nodeDecls, edgeStmts});
}

Result<NodeDeclaration> Parser::parseNodeDecl() {
    TRY_VOID(consumeToken(TokenTag::NODE,
                          "Node declaration starts with 'node' keyword"));
    TRY(nameRes, consumeIdentifier("A node declaration needs a name"));

    auto t = peek();
    if (t.getType() == TokenTag::STRINGLITERAL) {
        auto s = t.type.as_string();
        advance();
        return Result<NodeDeclaration>::ok({nameRes, NodeBody::simple(s)});
    } else if (t.getType() == TokenTag::LBRACE) {
        advance();

        TRY(bRes, parseExpNode());
        return Result<NodeDeclaration>::ok({nameRes, bRes});
    }
    return Result<NodeDeclaration>::err(
        "Parser",
        "SyntaxErr: Node declaration doesnt match the required syntax");
}

Result<NodeBody> Parser::parseExpNode() {
    std::vector<NodeField> ndFields;
    while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
        // advance();
        if (peek().getType() == TokenTag::TITLE) {
            advance(); // title itself
            TRY_VOID(consumeToken(TokenTag::COLON, "Need a colon after title"));
            TRY(titleRes, consumeString("Titles need a string value defined"));

            auto nd = NodeField::title(titleRes);
            ndFields.emplace_back(nd);
        } else if (peek().getType() == TokenTag::STYlE) {
            advance(); // style keyword
            StyleFields sf;

            TRY_VOID(consumeToken(TokenTag::LBRACE,
                                  "Need a starting brace after style keyword"));
            while (!isAtEnd() && !matchesType(TokenTag::RBRACE)) {
                auto currentToken = peek();
                auto current = currentToken.getType();

                if (current == TokenTag::FILL) {
                    advance(); // fill
                    TRY_VOID(consumeToken(TokenTag::COLON,
                                          "Need a colon after fill keyword"));
                    // TODO: will this drop an exception in case its not a
                    // bool?
                    auto val = advance().type.as_bool();
                    auto fl = Style::filling(val);
                    sf.emplace_back(fl);
                } else if (current == TokenTag::COLOR) {
                    advance(); // color
                    TRY_VOID(consumeToken(TokenTag::COLON,
                                          "Need a colon after color keyword"));
                    // TODO: will this drop an exception in case its not a
                    // string?
                    auto val = advance().type.as_string();
                    auto vl = Style::color(val);
                    sf.emplace_back(vl);
                } else if (current == TokenTag::SHAPE) {
                    advance(); // shape
                    TRY_VOID(consumeToken(TokenTag::COLON,
                                          "Need a colon after shape keyword"));
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
                        return Result<NodeBody>::err(
                            "Parser", "Shape style field supports either "
                                      "'Circle' or 'Square'");
                    }
                } else {
                    return Result<NodeBody>::err(
                        "Parser", "Style field supports either "
                                  "'Shape','Color', or 'Filling'");
                }
            }

            TRY_VOID(consumeToken(TokenTag::RBRACE,
                                  "A Node Style block should end with '}'"));
            ndFields.emplace_back(NodeField::style(sf));
        }
    }

    TRY_VOID(consumeToken(TokenTag::RBRACE,
                          "A Node declaration block should end with '}'"));
    return Result<NodeBody>::ok(NodeBody::expanded(ndFields));
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
Result<EdgeNode> Parser::parseEdgeStmt() {
    // two cases, one which is unlabeled
    // and one where the edge has a label
    // we have a node with its name used in edgenode
    // this node will then branch into edges which is saved inside edges as a
    // vector
    std::vector<Edge> edges;

    TRY(identifierRes,
        consumeIdentifier("Edge statements start with a node identifier"));
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
            advance(); // consume the literal)
            TRY_VOID(consumeToken(TokenTag::ARROW,
                                  "Expected an arrow after an identifier"));
            TRY(nextRes, consumeIdentifier(
                             "Expected a node identifier after edge arrow"));
            edges.emplace_back(identifierRes, label);
            identifierRes = nextRes;

        } else if (matchesType(TokenTag::IDENTIFIER)) {
            // bare edge: ident -> next
            edges.emplace_back(identifierRes);
            TRY(newIdentRes, consumeIdentifier("Expected node identifer"));
            identifierRes = newIdentRes;
        } else {
            return Result<EdgeNode>::err(
                "Parser", "Expected an identifer or string literal after '->'");
        }
    }
    edges.emplace_back(identifierRes);
    return Result<EdgeNode>::ok(EdgeNode(edges, false));
}
