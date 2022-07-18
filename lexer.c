#include "lexer.h"

/*================================================================/
* Forwarded declarations.
*=================================================================*/
void initLexer(const char* input);
char* substr(const char* source, int start, int endPos);
char* extractLiteral(Position pos);
void readChar();
static char peekChar();
static Token newTokenSymbol(TokenType type);
static TokenType lookupIdent(Position pos);
static bool isAlpha(char ch);
static bool isLetter(char ch);
static Position readIdentifier();
static bool isDigit(char ch);
static Position readNumber();
static Position readString();
static bool isSpace(char ch);
static void skipWhitespace();
Token nextToken();
void freeLexer();

Lexer l;

/*================================================================/
* Implementation
*=================================================================*/
void initLexer(const char* input) {
    l.position = 0;
    l.readPosition = 0;
    l.ch = 0;
    l.input = NULL;
    l.input = input;
    readChar(); // prime character.
}

char* extractLiteral(Position pos) {
    if (pos.start + pos.end == 0)
        return NULL;
    
    int len = pos.end - pos.start;
    char* literal = (char*)malloc(len);
    if (literal == NULL) {
        fprintf(stderr, "ERROR: not enough memory.\n");
        exit(74);
    }
    memcpy_s(literal, len, l.input + pos.start, len);
    literal[len] = '\0';
    if (strlen(literal) != len) {
        fprintf(stderr, "ERROR: strings does not match.\n");
        exit(74);
    }
    return literal; // the caller is responsible for freeing this variable.    
}

char* substr(const char* source, int start, int endPos) {
    char* result = (char*)malloc(endPos + 1);
    if (result == NULL) {
        fprintf(stderr, "ERROR: not enough memory.\n");
        exit(74);
    }
    memcpy_s(result, endPos, source + start, endPos);
    result[endPos] = '\0';
    if (strlen(result) != endPos) {
        fprintf(stderr, "ERROR: strings does not match.\n");
        exit(74);
    }
    return result; // the caller is responsible for freeing this variable.
}

void readChar() {
    if (l.readPosition > strlen(l.input)) {
        l.ch = '\0';
    } else {
        l.ch = l.input[l.readPosition];
    }
    l.position = l.readPosition;
    l.readPosition += 1;
}

static char peekChar() {
    if (l.readPosition >= strlen(l.input)) {
        return 0;
    }
    return l.input[l.readPosition];
}

static Token newTokenSymbol(TokenType type) {
    Token t;
    t.type = type;
    t.position.start = 0;
    t.position.end = 0;

    return t;
}

static TokenType lookupIdent(Position pos) {
    int len = pos.end - pos.start;
    
    // construir la palabra
    char word[len];
    int j=0;
    for (int i=pos.start; i < pos.end; i++) {
        word[j] = l.input[i];
        j++;
    }
    word[len] = '\0';

    // determinar si word es una palabra reservada o identificador
    if (strcmp(word, "fn") == 0) return T_FUNCTION;
    if (strcmp(word, "let") == 0) return T_LET;
    if (strcmp(word, "true") == 0) return T_TRUE;
    if (strcmp(word, "false") == 0) return T_FALSE;
    if (strcmp(word, "null") == 0) return T_NULL;
    if (strcmp(word, "if") == 0) return T_IF;
    if (strcmp(word, "else") == 0) return T_ELSE;
    if (strcmp(word, "return") == 0) return T_RETURN;

    return T_IDENT;
}

static bool isAlpha(char ch) {
    return isLetter(ch) || isDigit(ch);
}

static bool isLetter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

static Position readIdentifier() {
    Position pos;
    pos.start = l.position;
    do {
        readChar();
    } while (isAlpha(l.ch));
    pos.end = l.position;

    //return substr(l.input, position, l.position - position);
    return pos;
}

static bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

static Position readNumber() {
    Position pos;
    pos.start = l.position;
    do {
        readChar();
    } while (isDigit(l.ch));

    pos.end = l.position;

    return pos;
    // return substr(l.input, position, l.position - position);
}

static Position readString() {
    Position pos;    
    pos.start = l.position + 1; // pasada la comilla doble

    for (;;) {
        readChar();
        if (l.ch == '"' || l.ch == 0) {
            break;
        }
    }
    pos.end = l.position;

    return pos;
    // return substr(l.input, position, (l.position) - position);
}

static bool isSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static void skipWhitespace() {
    while (isSpace(l.ch)) {
        readChar();
    }
}

Token nextToken() {
    Token tok;
    tok.position.start = 0;
    tok.position.end = 0;

    skipWhitespace();
    switch (l.ch) {
    case '=':
        if (peekChar() == '=') {
            readChar();
            tok.type = T_EQ;
        } else {
            tok = newTokenSymbol(T_ASSIGN);
        }
        break;
    case '+':
        tok = newTokenSymbol(T_PLUS); break;
    case '-':
        tok = newTokenSymbol(T_MINUS); break;
    case '!':
        if (peekChar() == '=') {
            readChar();
            tok.type = T_NOT_EQ;            
        } else {
            tok = newTokenSymbol(T_BANG);
        }
        break;
    case '/':
        tok = newTokenSymbol(T_SLASH); break;
    case '*':
        tok = newTokenSymbol(T_ASTERISK); break;
    case '<':
        tok = newTokenSymbol(T_LT); break;
    case '>':
        tok = newTokenSymbol(T_GT); break;
    case ';':
        tok = newTokenSymbol(T_SEMICOLON); break;
    case ',':
        tok = newTokenSymbol(T_COMMA); break;
    case '(':
        tok = newTokenSymbol(T_LPAREN); break;
    case ')':
        tok = newTokenSymbol(T_RPAREN); break;
    case '{':
        tok = newTokenSymbol(T_LBRACE); break;
    case '}':
        tok = newTokenSymbol(T_RBRACE); break;
    case '"': 
        tok.type = T_STRING;
        tok.position = readString();
        break;
    case 0: 
        tok.type = T_EOF;
        break;    
    default:
        if (isLetter(l.ch)) {
            tok.position = readIdentifier();
            tok.type = lookupIdent(tok.position);
            return tok;
        } else if (isDigit(l.ch)) {
            tok.type = T_INT;
            tok.position = readNumber();
            return tok;
        } else {
            tok = newTokenSymbol(T_ILLEGAL);
        }
    }
    readChar();
    return tok;
}