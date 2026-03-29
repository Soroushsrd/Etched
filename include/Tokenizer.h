/**
 * @file Tokenizer.h
 * @brief Declaration of Tokenizer class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#pragma once

#include "Result.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

enum class TokenTag {
    GRAPH,
    NODE,
    TITLE,
    STYlE,
    FILL,
    SHAPE,
    COLOR,
    CIRCLE,
    SQUARE,
    ARROW,
    AND,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    COLON,
    SEMICOLON,
    COMMA,
    IDENTIFIER,
    STRINGLITERAL,
    NUMBER,
    BOOL,
    END,
};

using TokenData = std::variant<std::monostate, std::string, double, bool>;

class TokenType {
  private:
    TokenTag tag;
    TokenData data;
    TokenType(TokenTag t, TokenData d) : tag(t), data(std::move(d)) {}

  public:
    static TokenType identifier(std::string s) {
        return {TokenTag::IDENTIFIER, std::move(s)};
    }
    static TokenType number(double d) { return {TokenTag::NUMBER, d}; }
    static TokenType boolean(bool b) { return {TokenTag::BOOL, b}; }
    static TokenType simple(TokenTag t) { return {t, std::monostate{}}; }
    static TokenType literal(std::string s) {
        return {TokenTag::STRINGLITERAL, std::move(s)};
    }

    TokenTag get_tag() const { return tag; }
    const std::string &as_string() const { return std::get<std::string>(data); }
    double as_number() const { return std::get<double>(data); }
    bool as_bool() const { return std::get<bool>(data); }
};

struct Token {
    TokenType type;
    size_t line;
    size_t column;

    Token(TokenType tt, size_t ln, size_t cn)
        : type(std::move(tt)), line(ln), column(cn) {}

    TokenTag getType() const { return type.get_tag(); }
};

class Tokenizer {
  public:
    Tokenizer() = default;
    Tokenizer(const std::string &source)
        : sourceChars(source.begin(), source.end()), sourceString(source) {}
    Result<std::vector<Token>> Tokenize();

  private:
    size_t start = 0;
    size_t current = 0;
    size_t column = 1;
    size_t startColumn = 1;
    size_t line = 1;
    std::vector<char> sourceChars;
    std::string sourceString;
    std::vector<Token> tokens;

    static TokenType getKeyword(const std::string &txt) {
        if (txt == "graph")
            return TokenType::simple(TokenTag::GRAPH);
        if (txt == "node")
            return TokenType::simple(TokenTag::NODE);
        if (txt == "title")
            return TokenType::simple(TokenTag::TITLE);
        if (txt == "style")
            return TokenType::simple(TokenTag::STYlE);
        if (txt == "color")
            return TokenType::simple(TokenTag::COLOR);
        if (txt == "fill")
            return TokenType::simple(TokenTag::FILL);
        if (txt == "shape")
            return TokenType::simple(TokenTag::SHAPE);
        if (txt == "circle")
            return TokenType::simple(TokenTag::CIRCLE);
        if (txt == "square")
            return TokenType::simple(TokenTag::SQUARE);
        if (txt == "true")
            return TokenType::boolean(true);
        if (txt == "false")
            return TokenType::boolean(false);

        return TokenType::identifier(txt);
    }
    bool isAtEnd() const { return current >= sourceString.size(); }
    void addToken(TokenType tt);
    const std::string &getCurrentLine() const;
    char peekNext() const;
    char peek() const;
    char advance();
    bool matches(char input);
    void handleDigit();
    void handleIdentifier();
    VoidResult handleString();
    VoidResult scanTokens();
};

std::ostream &operator<<(std::ostream &os, const TokenType &tt);
// std::ostream &operator<<(std::ostream &os, const TokenTag &tt);
void printToken(Token &token);
