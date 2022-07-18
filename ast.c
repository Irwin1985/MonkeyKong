#include "ast.h"

/****************************************************************
* Rutinas para imprimir la estructura de los nodos.
*****************************************************************/
static char* printExpression(Expression* exp) {
	char* output = (char*)malloc(sizeof(char)*1024);
	if (output == NULL) {
		fprintf(stderr, "ERROR: not enough memory!.\n");
		exit(74);
	}
    switch (exp->type) {
	case NT_IDENT:
		sprintf_s(output, 1024, "%s", "policia extraer literal.");
		break;
	case NT_INTEGER:
		sprintf_s(output, 1024, "%i", ((IntegerNode*)exp->node)->value);
		break;
	case NT_BOOLEAN:
		sprintf_s(output, 1024, "%s", (((BooleanNode*)exp->node)->value == true) ? "true" : "false");
		break;
	case NT_STRING:
		sprintf_s(output, 1024, "\"%s\"", ((StringNode*)exp->node)->value);
		break;
	case NT_NULL:
		sprintf_s(output, 1024, "%s", "null");
		break;
	case NT_PREFIX: 
		{
			PrefixNode* prefix = ((PrefixNode*)exp->node);
			sprintf_s(output, 1024, "(%c%s)", prefix->operator, printExpression(prefix->right));
			break;
		}
	case NT_INFIX:
		{
			InfixNode* infix = ((InfixNode*)exp->node);
			sprintf_s(output, 1024, "(%s %s %s)", printExpression(infix->left), infix->operator, printExpression(infix->right));
			break;
		}
    }
    return output;
}

static char* printStatement(Statement* stmt) {
	char* output = (char*)malloc(sizeof(char)*1024);
	if (output == NULL) {
		fprintf(stderr, "ERROR: not enough memory!.\n");
		exit(74);
	}
    switch (stmt->type) {
	case NT_LET:
		{
			LetStatement* letStmt = ((LetStatement*)stmt->node);
			sprintf_s(output, 1024, "let %s = %s;", "<<policia extraer literal>>", printExpression(letStmt->value));
			break;
		}
	case NT_RETURN:
		{
			ReturnStatement* returnStmt = ((ReturnStatement*)stmt->node);
			sprintf_s(output, 1024, "return %s;", printExpression(returnStmt->value));
			break;
		}
	case NT_EXPR:
		sprintf_s(output, 1024, "%s;", printExpression(((ExpressionStatement*)stmt->node)->expression));
		break;
    }
    return output;
}

void printAST(ArrayStmt* program) {
	for (int i = 0; i < program->count; i++) {
		char* output = printStatement(program->statements[i]);
		fprintf(stdout, "%s\n", output);
		free(output);
	}
}

/****************************************************************
* Rutinas para liberar la memoria de los nodos.
*****************************************************************/
static void freeExpression(Expression* exp) {
    switch (exp->type) {
	case NT_IDENT:
		break;
    }
}

static void freeStatement(Statement* stmt) {
    switch (stmt->type) {
	case NT_LET: 
		{
			LetStatement* letStmt = ((LetStatement*)stmt->node);
			if (letStmt->name != NULL) free(letStmt->name);
			if (letStmt->value != NULL) freeExpression(letStmt->value);
			free(letStmt);
			break;
		}
	case NT_RETURN: 
		{
			ReturnStatement* returnStmt = ((ReturnStatement*)stmt->node);
			if (returnStmt->value != NULL) freeExpression(returnStmt->value);
			free(returnStmt);
			break;
		}		
	case NT_EXPR: 
		{
			ExpressionStatement* expStmt = ((ExpressionStatement*)stmt->node);
			if (expStmt->expression != NULL) freeExpression(expStmt->expression);
			free(expStmt);
			break;
		}
    }
}

void clearArrayStmt(ArrayStmt* array) {
	array->capacity = 0;
	array->count = 0;
	array->statements = NULL;	
}

void freeProgram(ArrayStmt* program) {
	for (int i = 0; i < program->count; i++) {
		if (program->statements[i] != NULL) {
			freeStatement(program->statements[i]);
		}
	}
	clearArrayStmt(program);

	free(program);
}