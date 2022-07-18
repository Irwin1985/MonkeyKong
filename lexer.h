#ifndef cmonk_lexer_h
#define cmonk_lexer_h

#include "headers.h"

typedef enum {
    T_ILLEGAL,
    T_EOF,

    // Identifiers + literals
    T_IDENT, // add, foobar, x, y, ...
    T_INT, // 1343456
    T_STRING, // "foo", "bar"

    // Operators
    T_ASSIGN,
    T_PLUS,
    T_MINUS,
    T_BANG,
    T_ASTERISK,
    T_SLASH,

    T_LT,
    T_GT,
    T_EQ,
    T_NOT_EQ,

    // Delimiters
    T_COMMA,
    T_SEMICOLON,

    T_LPAREN,
    T_RPAREN,
    T_RBRACE,
    T_LBRACE,

    // Keywords
    T_FUNCTION,
    T_LET,
    T_TRUE,
    T_FALSE,
    T_NULL,
    T_IF,
    T_ELSE,
    T_RETURN,
} TokenType;

static const char* tokenNames[] = {
    "T_ILLEGAL",
    "T_EOF",

    // Identifiers + literals
    "T_IDENT", // add, foobar, x, y, ...
    "T_INT", // 1343456
    "T_STRING", // "foo", "bar"

    // Operators
    "T_ASSIGN",
    "T_PLUS",
    "T_MINUS",
    "T_BANG",
    "T_ASTERISK",
    "T_SLASH",

    "T_LT",
    "T_GT",
    "T_EQ",
    "T_NOT_EQ",

    // Delimiters
    "T_COMMA",
    "T_SEMICOLON",

    "T_LPAREN",
    "T_RPAREN",
    "T_RBRACE",
    "T_LBRACE",

    // Keywords
    "T_FUNCTION",
    "T_LET",
    "T_TRUE",
    "T_FALSE",
    "T_NULL",
    "T_IF",
    "T_ELSE",
    "T_RETURN"
};

typedef struct {
    int start;
    int end;
} Position;

typedef struct {
    TokenType type;
    Position position;
    //void* literal;
} Token;

typedef struct {    
    const char* input;
    int position; // current position in input (points to current char)
    int readPosition; // current readint position in input (after current char)
    char ch; // current char under examination
} Lexer;

/*================================================================/
* PUBLIC LEXER API
*=================================================================*/
void initLexer(const char* input);
char* extractLiteral(Position pos);
Token nextToken();

#endif