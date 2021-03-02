#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

void token_free(token_t* tok){
	if (tok->type != END_OF_FILE) {
		free(tok->literal);
	}
	else {
		free(tok);
	}
}

/*
* Verifica si el contenido de 'literal' es numérico. ej: 12345
* recorre el puntero n veces y rompe el bucle si un elemento no es digito.
*/
static int is_number(char* literal) {
	while (1) {
		char c = *literal;
		if (!c){
			break;
		}
		if (!isdigit(c)){
			return 0;
		}
		literal++;
	}
	return 1;
}

token_type get_token_type(char* literal) {
	if (strcmp(literal, "let") == 0) {
		return LET;
	}
	if (strcmp(literal, "fn") == 0) {
		return FUNCTION;
	}
	if (strcmp(literal, "if") == 0) {
		return IF;
	}
	if (strcmp(literal, "else") == 0) {
		return ELSE;
	}
	if (strcmp(literal, "return") == 0) {
		return RETURN;
	}
	if (strcmp(literal, "true") == 0) {
		return TRUE;
	}
	if (strcmp(literal, "false") == 0) {
		return FALSE;
	}
	if (strcmp(literal, "while") == 0) {
		return WHILE;
	}
	if (is_number(literal)) {
		return INT;
	}
	return IDENT;
}

token_t* token_copy(token_t* src) {
	token_t* copy = malloc(sizeof(*copy));
	if (copy == NULL) {
		return NULL;
	}
	copy->type = src->type;
	copy->literal = strdup(src->literal);
	if (copy->literal == NULL) {
		free(copy);
		return NULL;
	}
	return copy;
}