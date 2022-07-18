#ifndef cmonk_interpreter_h
#define cmonk_interpreter_h

#define GC_MAX_OBJECTS 1024 * 1024

#include <stdarg.h>
#include "parser.h"
#include "object.h"

void initEvaluator();
void freeEvaluator();
void gc();

/*================================================================/
* PUBLIC INTERPRETER API
*=================================================================*/
Object* interpret(const char* source);
Object* evalProgram(ArrayStmt* program, Environment* env);
Object* evalStatements(Statement* stmt, Environment* env);
Object* evalExpression(Expression* exp, Environment* env);

#endif