#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type {
	ILLEGAL,
	END_OF_FILE,

	// identifiers, literals
	IDENT,
	INT,
	STRING,

	// operators
	ASSIGN,
	PLUS,
	MINUS,
	BANG,
	SLASH,
	ASTERISK,
	PERCENT,
	LT,
	GT,
	EQ,
	NOT_EQ,
	AND,
	OR,

	// delimiters
	COMMA,
	SEMICOLON,
	LPAREN,
	RPAREN,
	LBRACE,
	RBRACE,
	LBRACKET,
	RBRACKET,
	COLON,

	// keywords
	FUNCTION,
	LET,
	IF,
	ELSE,
	RETURN,
	TRUE,
	FALSE,
	WHILE
} token_type;

static const char* token_names[] = {
	"ILLEGAL",
	"END_OF_FILE",

	// identifiers, literals
	"IDENT",
	"INT",
	"STRING",

	// operator
	"ASSIGN",
	"PLUS",
	"MINUS",
	"BANG",
	"SLASH",
	"ASTERISK",
	"PERCENT",
	"LT",
	"GT",
	"EQ",
	"NOT_EQ",
	"AND",
	"OR",

	// delimiters
	"COMMA",
	"SEMICOLON",
	"LPAREN",
	"RPAREN",
	"LBRACE",
	"RBRACE",
	"LBRACKET",
	"RBRACKET",
	"COLON",

	// keywords
	"FUNCTION",
	"LET",
	"IF",
	"ELSE",
	"RETURN",
	"TRUE",
	"FALSE",
	"WHILE"
};

#define get_token_name(tok) token_names[tok->type]
#define get_token_name_from_type(tok_type) token_names[tok_type]

typedef struct token_t {
	token_type type;
	char* literal;
} token_t;

void token_free(token_t*);
token_t* token_copy(token_t*);
token_type get_token_type(char*);

#endif