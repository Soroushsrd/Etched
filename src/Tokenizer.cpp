/**
 * @file Tokenizer.cpp
 * @brief Implementation of Tokenizer class
 * @author Soroosh Sardashti <sardashtisoroosh@gmail.com>
 * @date 2026-03-08
 */

#include "Tokenizer.h"
#include <cctype>
#include <iostream>
#include <string>

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
    // std::string txt{sourceChars.begin() + start, sourceChars.begin() +
    // current};
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

void Tokenizer::handleString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 1;
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
        std::cerr << "Unterminated string literal\n";
        return;
    }

    std::string txt(sourceChars.begin() + start + 1,
                    sourceChars.begin() + current);
    // "
    advance();

    addToken(TokenType::literal(txt));
}

void Tokenizer::scanTokens() {
    char c = advance();

    switch (c) {
    case '-': {
        if (matches('>')) {
            addToken(TokenType::simple(TokenTag::ARROW));
        } else {
            std::cerr << "Missing '>'\n";
        }

        return;
    }
    case '&': {
        if (matches('&')) {
            addToken(TokenType::simple(TokenTag::AND));
        } else {
            std::cerr << "Missing '&'\n";
        }
        return;
    }
    case '{': {
        addToken(TokenType::simple(TokenTag::LBRACE));
        return;
    }

    case '}': {
        addToken(TokenType::simple(TokenTag::RBRACE));
        return;
    }
    case '(': {
        addToken(TokenType::simple(TokenTag::LPAREN));
        return;
    }
    case ')': {
        addToken(TokenType::simple(TokenTag::RPAREN));
        return;
    }
    case ',': {
        addToken(TokenType::simple(TokenTag::COMMA));
        return;
    }
    case ':': {
        addToken(TokenType::simple(TokenTag::COLON));
        return;
    }
    case ';': {
        addToken(TokenType::simple(TokenTag::SEMICOLON));
        return;
    }
    case '"': {
        handleString();
        return;
    }
    case ' ': {
        return;
    }
    case '\t': {
        column += 3;
        return;
    }
    case '\r': {
        return;
    }
    case '\n': {
        line++;
        column = 1;
        return;
    }
    case '/': {
        if (matches('/')) {
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
        } else {
            std::cerr << "Missing '/'\n";
        }
        return;
    }
    default: {
        if (std::isdigit(c)) {
            handleDigit();
        } else if (std::isalpha(c)) {
            handleIdentifier();
        } else {
            std::cerr << "Unkwnown Token: " << c << std::endl;
        }
        return;
    }
    }
}

void Tokenizer::Tokenize() {
    while (!isAtEnd()) {
        start = current;
        startColumn = column;
        scanTokens();
    }

    addToken(TokenType::simple(TokenTag::END));
}

void printToken(Token &token) {
    std::cout << "Type: " << token.type << ", line: " << token.line
              << ", column: " << token.column << std::endl;
}
// TODO: getCurrentLine
