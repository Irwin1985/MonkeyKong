#ifndef cmonk_ast_h

#define cmonk_ast_h

#define FIRST_ARRAY_CAPACITY 8
#define GROWING_ARRAY_FACTOR 2

#include "lexer.h"

/**
 * Estructura y funcionamiento del AST:
 * La estructura está formada por 2 wrappers:
 * 1. Statement: contiene las sentencias soportadas por el lenguaje.
 * 	a. LetStatement: 	let numero = 7;
 * 	b. ReturnStatement: return 7;
 * 	c. ExpressionStatement: una expresión que se representa a sí misma. e.g: 7;
 * 
 * 2. Expression: contiene todas las expresiones del lenguaje.
 * 	a. IntegerNode: 3
 * 	b. BooleanNode: true | false
 * 	c. StringNode: "bar"
 * 	d. IdentifierNode: foo
 * 	e. NullNode: null
 * 	f. PrefixNode: -5
 * 	g. InfixNode: a + b
 * 	h. IfNode: if (a) {...} else {...}
 * 
 * Proceso: cada nodo generado por el Parser deberá ser envuelto en su respectivo wrapper
 * por ejemplo:
 * a. Si la regla de producción es un LetStatement entonces se crea el wrapper Statement con
 * tipo NT_LET y su representación en memoria sería la siguiente:
 * 
 * Statement {
 * 	.type = NT_LET,
 * 	.node = LetStatement {...}
 * }
 * b. Si el Parser detecta la expresión a + b entonces el nodo final será un Expression:
 * Expression {
 * 	.type = NT_INFIX,
 * 	.node = InfixExpression {...}
 * }
 * 
 * Al estar todos los nodos envueltos en un wrapper se hace fácil recorrerlos e
 * interpretarlos ya que solo hay que evaluar el tipo de Statement y Expression.
 * tras ser detectados se procede a 'desemvolver' el nodo.
 */

// el tipo de nodo es clave para saber el objeto que se está examinando.
typedef enum {
	NT_EXPR,
	NT_LET,
	NT_RETURN,
	NT_IDENT,
	NT_INTEGER,
	NT_STRING,
	NT_BOOLEAN,
	NT_NULL,
	NT_PREFIX,
	NT_INFIX,
	NT_IF,
	NT_FUNCTION,
	NT_CALL,
} NodeType;

// Statement ::= letStatement | returnStatement | expressionStatement
typedef struct {
	NodeType type;
	void* node;
} Statement;

// Expression ::= Integer | Boolean | String | Identifier | Null | Prefix | Infix
typedef struct {
	NodeType type;
	void* node;
} Expression;

// ArrayStmt: contiene un array de sentencias (sirve para Program y BlockStatement)
typedef struct {
	int count;
	int capacity;
	Statement **statements;
} ArrayStmt;

/**********************************************************
* Nodos para Expresiones
***********************************************************/
// Nodo IntegerNode
typedef struct {
	Token token;
	int value;
} IntegerNode;

// Nodo BooleanNode
typedef struct {
	Token token;
	bool value;
} BooleanNode;

// Nodo NullNode
typedef struct {
	Token token;
} NullNode;

// Nodo StringNode
typedef struct {
	Token token;
	char *value;
} StringNode;

// Nodo IdentifierNode
typedef struct {
	Token token;
	//char *value;
} IdentifierNode;

// Nodo ExpressionStatement
typedef struct {
	Token token;
	Expression* expression;
} ExpressionStatement;

// Nodo PrefixNode
typedef struct {
	Token token;
	TokenType operator;
	Expression* right;
} PrefixNode;

// Nodo InfixNode
typedef struct {
	Token token;
	Expression* left;
	TokenType operator;
	Expression* right;
} InfixNode;

// Nodo IfNode
typedef struct {
	Token token;
	Expression* condition;
	ArrayStmt* consequence;
	ArrayStmt* alternative;
} IfNode;

/**********************************************************
* Nodos para Sentencias o Statements
***********************************************************/
// Nodo LetStatement
typedef struct {
	IdentifierNode* name;
	Expression* value;
} LetStatement;

// Nodo ReturnStatement
typedef struct {
	Token token;
	Expression* value;
} ReturnStatement;

// Nodo FunctionNode
typedef struct {
	IdentifierNode* parameters[255];
	int arity;
	ArrayStmt* body;
} FunctionNode;

// Nodo CallNode
typedef struct {
	Expression* function;
	Expression* arguments[255];
	int argc;
} CallNode;

void printAST(ArrayStmt* program);
void clearArrayStmt(ArrayStmt* array);
void freeProgram(ArrayStmt* program);

#endif