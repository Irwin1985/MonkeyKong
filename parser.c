#include "parser.h"

/*================================================================/
* Forwarded declarations.
*=================================================================*/
static void advance();
static bool curTokenIs(TokenType t);
static bool match(TokenType t);
static ArrayStmt* newArray();
static Expression* newExpression(NodeType type, void* node);
static Statement* newStatement(NodeType type, void* node);
Expression* parseIdentifier();
Expression* parseIntegerLiteral();
Expression* parseBooleanLiteral();
Expression* parseStringLiteral();
static Precedence curPrecedence();
static Expression* parseExpression(Precedence pre);
Expression* parsePrefixExpression();
Expression* parseInfixExpression(Expression* left);
Expression* parseGroupedExpression();
Expression* parseNullLiteral();
static ArrayStmt* parseBlockStatement();
static IfNode* newIfNode(Expression* condition, ArrayStmt* consequence, ArrayStmt* alternative);
Expression* parseIfExpression();
static bool peekTokenIs(TokenType t);
static Statement* parseLetStatement();
static Statement* parseReturnStatement();
static Statement* parseExpressionStatement();
static Statement* parseStatement();
static Expression* parseFunctionLiteral();
static Expression* parseCallExpression(Expression* function);
void appendStatement(ArrayStmt* array, Statement* stmt);
ArrayStmt* parseProgram();

Parser p;
// array de tokens->funciones
static void* prefixParseFns[] = {
    NULL, // T_ILLEGAL
    NULL, // T_EOF
    parseIdentifier,        // T_IDENT
    parseIntegerLiteral,    // T_INT
    parseStringLiteral,     // T_STRING
    NULL, // T_ASSIGN
    NULL, // T_PLUS
    parsePrefixExpression, // T_MINUS
    parsePrefixExpression, // T_BANG
    NULL, // T_ASTERISK
    NULL, // T_SLASH
    NULL, // T_LT
    NULL, // T_GT
    NULL, // T_EQ
    NULL, // T_NOT_EQ
    NULL, // T_COMMA
    NULL, // T_SEMICOLON
    parseGroupedExpression, // T_LPAREN
    NULL, // T_RPAREN
    NULL, // T_RBRACE
    NULL, // T_LBRACE
    parseFunctionLiteral, // T_FUNCTION
    NULL, // T_LET
    parseBooleanLiteral, // T_TRUE
    parseBooleanLiteral, // T_FALSE
    parseNullLiteral, // T_NULL
    parseIfExpression, // T_IF
    NULL, // T_ELSE
    NULL // T_RETURN
};

static void* infixParseFns[] = {
    NULL, // T_ILLEGAL
    NULL, // T_EOF
    NULL, // T_IDENT
    NULL, // T_INT
    NULL, // T_STRING
    NULL, // T_ASSIGN
    parseInfixExpression, // T_PLUS
    parseInfixExpression, // T_MINUS
    NULL, // T_BANG
    parseInfixExpression, // T_ASTERISK
    parseInfixExpression, // T_SLASH
    parseInfixExpression, // T_LT
    parseInfixExpression, // T_GT
    parseInfixExpression, // T_EQ
    parseInfixExpression, // T_NOT_EQ
    NULL, // T_COMMA
    NULL, // T_SEMICOLON
    parseCallExpression, // T_LPAREN
    NULL, // T_RPAREN
    NULL, // T_RBRACE
    NULL, // T_LBRACE
    NULL, // T_FUNCTION
    NULL, // T_LET
    NULL, // T_TRUE
    NULL, // T_FALSE
    NULL, // T_NULL
    NULL, // T_IF
    NULL, // T_ELSE
    NULL // T_RETURN
};

static int precedences[] = {
    0, // T_ILLEGAL
    0, // T_EOF
    0, // T_IDENT
    0, // T_INT
    0, // T_STRING
    0, // T_ASSIGN
    SUM, // T_PLUS
    SUM, // T_MINUS
    0, // T_BANG
    PRODUCT, // T_ASTERISK
    PRODUCT, // T_SLASH
    LESSGREATER, // T_LT
    LESSGREATER, // T_GT
    EQUALS, // T_EQ
    EQUALS, // T_NOT_EQ
    0, // T_COMMA
    0, // T_SEMICOLON
    CALL, // T_LPAREN
    0, // T_RPAREN
    0, // T_RBRACE
    0, // T_LBRACE
    0, // T_FUNCTION
    0, // T_LET
    0, // T_TRUE
    0, // T_FALSE
    0, // T_NULL
    0, // T_IF
    0, // T_ELSE
    0 // T_RETURN
};

/*================================================================/
* Implementation
*=================================================================*/
static void advance() {
	// if (p.curToken.literal != NULL) 
	// 	free(p.curToken.literal);

	p.curToken = p.peekToken;
	p.peekToken = nextToken();
}

static bool curTokenIs(TokenType t) {
	return p.curToken.type == t;
}

static bool match(TokenType t) {
	if (curTokenIs(t)) {
		advance();
		return true;
	}
	return false;
}

static ArrayStmt* newArray() {
	ArrayStmt* array = createObject(ArrayStmt);

	// Inicializar los campos del array.
	clearArrayStmt(array);

	return array;
}

static Expression* newExpression(NodeType type, void* node) {
	Expression* exp = createObject(Expression);	

	exp->type = type;
	exp->node = node;

	return exp;
}

static Statement* newStatement(NodeType type, void* node) {
	Statement* stmt = createObject(Statement);
	stmt->type = type;
	stmt->node = node;

	return stmt;
}

Expression* parseIdentifier() {
	IdentifierNode* node = createObject(IdentifierNode);
	node->token = p.curToken;
	// node->value = strdup(p.curToken.literal);

	advance(); // advance T_IDENT

	return newExpression(NT_IDENT, node);
}

Expression* parseIntegerLiteral() {
	IntegerNode* node = createObject(IntegerNode);
	node->token = p.curToken;
	// node->value = atoi(p.curToken.literal);

	advance();

	return newExpression(NT_INTEGER, node);
}

Expression* parseBooleanLiteral() {
	BooleanNode* node = createObject(BooleanNode);
	node->token = p.curToken;
	node->value = p.curToken.type == T_TRUE ? true : false;

	advance();

	return newExpression(NT_BOOLEAN, node);
}

Expression* parseStringLiteral() {
	StringNode* node = createObject(StringNode);
	node->token = p.curToken;
	// node->value = strdup(p.curToken.literal);

	advance();

	return newExpression(NT_STRING, node);
}

static Precedence curPrecedence() {
	return precedences[p.curToken.type];
}

static Expression* parseExpression(Precedence pre) {
	prefixParseFn prefix = prefixParseFns[p.curToken.type];
	if (prefix == NULL) {
		return NULL;
	}
	Expression* leftExp = prefix();

	while (!curTokenIs(T_SEMICOLON) && pre < curPrecedence(p)) {
		infixParseFn infix = infixParseFns[p.curToken.type];
		if (infix == NULL) {
			return leftExp;
		}
		leftExp = infix(leftExp);
	}

	return leftExp;
}

Expression* parsePrefixExpression() {
	PrefixNode* node = createObject(PrefixNode);
	node->token = p.curToken;
	node->operator = p.curToken.type;

	advance(); // skip the PREFIX token

	node->right = parseExpression(PREFIX);

	return newExpression(NT_PREFIX, node);
}

Expression* parseInfixExpression(Expression* left) {
	InfixNode* node = createObject(InfixNode);	
	node->left = left;
	node->operator = p.curToken.type;

	Precedence pre = curPrecedence();
	advance(); // skip the INFIX token

	node->right = parseExpression(pre);

	return newExpression(NT_INFIX, node);
}

Expression* parseGroupedExpression() {	

	advance(); // skip LPAREN
	Expression* exp = parseExpression(LOWEST);

	if (!match(T_RPAREN)) {
		return NULL;
	}

	return exp;
}

Expression* parseNullLiteral() {
	NullNode* node = createObject(NullNode);
	node->token = p.curToken;

	advance(); // skip T_NULL

	return newExpression(NT_NULL, node);
}

static ArrayStmt* parseBlockStatement() {
	ArrayStmt* stmts = newArray();

	match(T_LBRACE);

	while (!curTokenIs(T_EOF) && !curTokenIs(T_RBRACE)) {
		Statement* stmt = parseStatement(p);
		if (stmt != NULL) {
			appendStatement(stmts, stmt);
		}
	}

	match(T_RBRACE);

	return stmts;
}

static IfNode* newIfNode(Expression* condition, ArrayStmt* consequence, ArrayStmt* alternative) {
	IfNode* node = createObject(IfNode);	
	node->condition = condition;
	node->consequence = consequence;
	node->alternative = alternative;
	
	return node;
}

Expression* parseIfExpression() {
	Token token = p.curToken;
	advance(); // skip T_IF
	
	match(T_LPAREN);
	Expression* condition = parseExpression(LOWEST);
	match(T_RPAREN);

	ArrayStmt* consequence = parseBlockStatement(p);
	ArrayStmt* alternative = NULL;

	if (curTokenIs(T_ELSE)) {
		advance(); // skip T_ELSE
		alternative = parseBlockStatement();
	}

	IfNode* node = newIfNode(condition, consequence, alternative);
	node->token = token;

	return newExpression(NT_IF, node);
}

static Expression* parseFunctionLiteral() {
	FunctionNode* node = createObject(FunctionNode);
	advance(); // skip T_FUNCTION

	// parameters
	if (!match(T_LPAREN)) return NULL;
	if (!curTokenIs(T_RPAREN)) {
		int counter = 0;
		node->parameters[counter++] = (IdentifierNode*)parseIdentifier()->node;
		while (!curTokenIs(T_EOF) && curTokenIs(T_COMMA)) {
			advance(); // T_COMMA
			if (counter == 255) {
				fprintf(stderr, "Can't have more than 255 parameters.");
				return NULL;
			}
			node->parameters[counter++] = (IdentifierNode*)parseIdentifier()->node;
		}
		node->arity = counter;
	}
	if (!match(T_RPAREN)) return NULL;

	node->body = parseBlockStatement();
	
	return newExpression(NT_FUNCTION, node);
}

static Expression* parseCallExpression(Expression* function) {
	CallNode* node = createObject(CallNode);
	node->function = function;

	advance(); // T_LPAREN

	if (!curTokenIs(T_RPAREN)) {
		int count = 0;
		node->arguments[count++] = parseExpression(LOWEST);
		while (!curTokenIs(T_EOF) && curTokenIs(T_COMMA)) {
			advance(); // T_COMMA
			node->arguments[count++] = parseExpression(LOWEST);
		}
		node->argc = count;
	}
	match(T_RPAREN);

	return newExpression(NT_CALL, node);
}

void initParser() {
	// p.curToken.literal = NULL;
	// p.peekToken.literal = NULL;
	advance();
	advance();
}

static bool peekTokenIs(TokenType t) {
	return p.peekToken.type == t;
}

static Statement* parseLetStatement() {	
	LetStatement* node = createObject(LetStatement);
	advance(); // skip T_LET

	if (!curTokenIs(T_IDENT)) {
		return NULL;
	}
	
	// creamos el nodo Identifier para que forme parte del name de LetStatement
	IdentifierNode* ident = createObject(IdentifierNode);
	ident->token = p.curToken;
	// ident->value = strdup(p.curToken.literal);
	
	advance(); // skip T_IDENT

	if (!match(T_ASSIGN)) {
		return NULL;
	}

	node->name = ident;
	node->value = parseExpression(LOWEST);

	if (curTokenIs(T_SEMICOLON)) {
		advance(); 
	}

	return newStatement(NT_LET, node);
}

static Statement* parseReturnStatement() {
	ReturnStatement* node = createObject(ReturnStatement);
	advance(); // skip RETURN	

	node->token = p.curToken;
	node->value = parseExpression(LOWEST);
	
	if (curTokenIs(T_SEMICOLON)) {
		advance(); 
	}

	return newStatement(NT_RETURN, node);
}

static Statement* parseExpressionStatement() {
	ExpressionStatement* node = createObject(ExpressionStatement);
	node->expression = parseExpression(LOWEST);

	if (curTokenIs(T_SEMICOLON)) {
		advance();
	}

	return newStatement(NT_EXPR, node);
}

static Statement* parseStatement() {
	switch (p.curToken.type) {
	case T_LET:
		return parseLetStatement();
	case T_RETURN:
		return parseReturnStatement();
	default:
		return parseExpressionStatement();
	}
}

// realiza la lógica de incrementar el array de sentencias.
void appendStatement(ArrayStmt* array, Statement* stmt) {
	if (array->capacity < (array->count+1)) {
		int cap = array->capacity;
		array->capacity   = (cap == 0) ? FIRST_ARRAY_CAPACITY : cap * 2;
		array->statements = realloc(array->statements, sizeof(Statement) * array->capacity);
	}
	// agregar la nueva sentencia al array.
	array->statements[array->count] = stmt;
	array->count += 1;
}

ArrayStmt* parseProgram() {
	initParser();
	ArrayStmt* program = newArray();

	while (!curTokenIs(T_EOF)) {
		Statement* stmt = parseStatement();
		if (stmt != NULL) {
			appendStatement(program, stmt);
		}
	}

	return program;
}