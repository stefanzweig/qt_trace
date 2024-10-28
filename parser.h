#ifndef PARSER_H
#define PARSER_H

/* Demo expressions 
 * expression ::= term (('+' | '-') term)*
 * term       ::= factor (('*' | '/') factor)*
 * factor     ::= NUMBER | '(' expression ')'
*/

/*
* Mine:
* expression ::= factor (| factor)*
* factor     ::= identifier (= identifier)*
*/
#include "lexer.h"

class Parser {
public:
    void parse() {
        expression();
    }

private:
    Lexer& lexer;
    Token currentToken;
    int idx = -1;

    void expression() 
    {
    }

    void term() 
    {
    }

    void factor() 
    {
    }
};


/*

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer), currentToken(lexer.nextToken()) {}

    void parse() {
        expression();
    }

private:
    Lexer& lexer;
    Token currentToken;

    void expression() {
        term();
        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
            currentToken = lexer.nextToken(); // consume operator
            term();
        }
    }

    void term() {
        factor();
        while (currentToken.type == TokenType::MULTIPLY || currentToken.type == TokenType::DIVIDE) {
            currentToken = lexer.nextToken(); // consume operator
            factor();
        }
    }

    void factor() {
        if (currentToken.type == TokenType::NUMBER) {
            currentToken = lexer.nextToken(); // consume number
        } else if (currentToken.type == TokenType::LPAREN) {
            currentToken = lexer.nextToken(); // consume '('
            expression();
            // expect a closing parenthesis
            if (currentToken.type == TokenType::RPAREN) {
                currentToken = lexer.nextToken(); // consume ')'
            } else {
                throw std::runtime_error("Expected ')'");
            }
        } else {
            throw std::runtime_error("Unexpected token");
        }
    }
};
*/
#endif // PARSER_H