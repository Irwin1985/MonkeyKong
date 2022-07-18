#ifndef cmonk_parser_h
#define cmonk_parser_h

#include "ast.h"

// Orden de precedencia para los operadores
typedef enum {
    LOWEST,         // nothing
    EQUALS,         // == or !=
    LESSGREATER,    // > or <
    SUM,            // + or -
    PRODUCT,        // * or /
    PREFIX,         // -x or !x
    CALL            // myFunction(x)
} Precedence;

typedef struct {
	Lexer* l;
	Token curToken;
	Token peekToken;
} Parser;


// functionPointer ::= typedef type (*functionName)(args)
typedef Expression* (*prefixParseFn)();
typedef Expression* (*infixParseFn)(Expression* left);

/*================================================================/
* PUBLIC PARSER API
*=================================================================*/
ArrayStmt* parseProgram();

#endif