/**
 * @file Tokenizer.cpp
 * @brief Implementation of Tokenizer class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#include "Tokenizer.h"
#include "Result.h"
#include <cctype>
#include <iostream>
#include <vector>

std::ostream &operator<<(std::ostream &os, TokenType &tt) {
    switch (tt.get_tag()) {
    case TokenTag::GRAPH:
        return os << "Graph";
    case TokenTag::NODE:
        return os << "Node";
    case TokenTag::TITLE:
        return os << "Title";
    case TokenTag::STYlE:
        return os << "Style";
    case TokenTag::FILL:
        return os << "Fill";
    case TokenTag::SHAPE:
        return os << "Shape";
    case TokenTag::COLOR:
        return os << "Color";
    case TokenTag::CIRCLE:
        return os << "Circle";
    case TokenTag::SQUARE:
        return os << "Square";
    case TokenTag::ARROW:
        return os << "Arrow";
    case TokenTag::AND:
        return os << "And";
    case TokenTag::LBRACE:
        return os << "LBrace";
    case TokenTag::RBRACE:
        return os << "RBrace";
    case TokenTag::LPAREN:
        return os << "LParen";
    case TokenTag::RPAREN:
        return os << "RParen";
    case TokenTag::COLON:
        return os << "Colon";
    case TokenTag::SEMICOLON:
        return os << "SemiColon";
    case TokenTag::COMMA:
        return os << "Comma";
    case TokenTag::IDENTIFIER:
        return os << "Identifier(" << tt.as_string() << ")";
    case TokenTag::STRINGLITERAL:
        return os << "StringLiteral(" << tt.as_string() << ")";
    case TokenTag::NUMBER:
        return os << "Number(" << tt.as_number() << ")";
    case TokenTag::BOOL:
        return os << "Bool(" << tt.as_bool() << ")";
    case TokenTag::END:
        return os << "Eof";
    }
    return os;
}

void Tokenizer::addToken(TokenType tt) {
    tokens.emplace_back(std::move(tt), line, startColumn);
}

char Tokenizer::peekNext() const {
    if (current + 1 >= sourceChars.size()) {
        return '\0';
    }
    return sourceChars.at(current + 1);
}

char Tokenizer::peek() const {
    if (!isAtEnd()) {
        return sourceChars.at(current);
    }
    return '\0';
}

char Tokenizer::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    char c{sourceChars.at(current)};
    current++;
    column++;
    return c;
}

bool Tokenizer::matches(char input) {
    if (isAtEnd()) {
        return false;
    }
    if (sourceChars.at(current) == input) {
        current++;
        column++;
        return true;
    }
    return false;
}

void Tokenizer::handleDigit() {
    while (std::isdigit(peek())) {
        advance();
    }
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance();
        while (std::isdigit(peek())) {
            advance();
        }
    }

    std::string numbertxt(sourceChars.begin() + start + 1,
                          sourceChars.begin() + current);
    auto number = std::stod(numbertxt);
    addToken(TokenType::number(number));
}

void Tokenizer::handleIdentifier() {
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string txt(sourceChars.begin() + start, sourceChars.begin() + current);
    addToken(getKeyword(txt));
}

VoidResult Tokenizer::handleString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            return VoidResult::err("Tokenizer", "Unterminated string literal",
                                   line, column);
        }

        if (peek() == '\\') {
            // '\'
            advance();
            if (!isAtEnd()) {
                advance();
                continue;
            }
        }
        advance();
    }

    if (isAtEnd()) {
        return VoidResult::err("Tokenizer", "Unterminated string literal", line,
                               column);
    }

    std::string txt(sourceChars.begin() + start + 1,
                    sourceChars.begin() + current);
    // "
    advance();

    addToken(TokenType::literal(txt));
    return VoidResult::ok();
}

VoidResult Tokenizer::scanTokens() {
    char c = advance();

    switch (c) {
    case '-': {
        if (matches('>')) {
            addToken(TokenType::simple(TokenTag::ARROW));
        } else {
            return VoidResult::err("Tokenizer", "Missing '>'", line, column);
        }

        return VoidResult::ok();
    }
    case '&': {
        if (matches('&')) {
            addToken(TokenType::simple(TokenTag::AND));
        } else {
            return VoidResult::err("Tokenizer", "Missing '&'", line, column);
        }
        return VoidResult::ok();
    }
    case '{': {
        addToken(TokenType::simple(TokenTag::LBRACE));
        return VoidResult::ok();
    }

    case '}': {
        addToken(TokenType::simple(TokenTag::RBRACE));
        return VoidResult::ok();
    }
    case '(': {
        addToken(TokenType::simple(TokenTag::LPAREN));
        return VoidResult::ok();
    }
    case ')': {
        addToken(TokenType::simple(TokenTag::RPAREN));
        return VoidResult::ok();
    }
    case ',': {
        addToken(TokenType::simple(TokenTag::COMMA));
        return VoidResult::ok();
    }
    case ':': {
        addToken(TokenType::simple(TokenTag::COLON));
        return VoidResult::ok();
    }
    case ';': {
        addToken(TokenType::simple(TokenTag::SEMICOLON));
        return VoidResult::ok();
    }
    case '"': {
        auto r = handleString();
        if (!r) {
            return r;
        }
        return VoidResult::ok();
    }
    case ' ': {
        return VoidResult::ok();
    }
    case '\t': {
        column += 3;
        return VoidResult::ok();
    }
    case '\r': {
        return VoidResult::ok();
    }
    case '\n': {
        line++;
        column = 1;
        return VoidResult::ok();
    }
    case '/': {
        if (matches('/')) {
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
        } else {
            return VoidResult::err("Tokenizer", "Missing '/'", line, column);
        }
        return VoidResult::ok();
    }
    default: {
        if (std::isdigit(c)) {
            handleDigit();
        } else if (std::isalpha(c)) {
            handleIdentifier();
        } else {
            return VoidResult::err(
                "Tokenizer", std::format("Unknown Token: {}", c), line, column);
        }
        return VoidResult::ok();
    }
    }
}

Result<std::vector<Token>> Tokenizer::Tokenize() {
    while (!isAtEnd()) {
        start = current;
        startColumn = column;
        auto r = scanTokens();
        if (!r) {
            std::cerr << r.error().format() << std ::endl;
            return Result<std::vector<Token>>::err(r.error());
        }
    }

    addToken(TokenType::simple(TokenTag::END));
    return Result<std::vector<Token>>::ok(std::move(tokens));
}

void printToken(Token &token) {
    std::cout << "Type: " << token.type << ", line: " << token.line
              << ", column: " << token.column << std::endl;
}
