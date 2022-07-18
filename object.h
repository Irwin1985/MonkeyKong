#ifndef cmonk_object_h
#define cmonk_object_h

// hash
#define HASHSIZE 101
// hash

#include "headers.h"
#include "ast.h"

/**
 * Funcionamiento del sistema de objetos.
 * Cada objeto del programa será envuelto en un wrapper tipo Object cuyo campo 'value'
 * contiene el objeto real.
 */

typedef enum {
    INTEGER_OBJ,
    STRING_OBJ,
    BOOLEAN_OBJ,
    NULL_OBJ,
    RETURN_OBJ,
    ERROR_OBJ,
    FUNCTION_OBJ,
} ObjectType;

typedef struct {
    int value;
} IntegerObj;

typedef struct {
    char* value;
} StringObj;

typedef struct {
    bool value;
} BooleanObj;

typedef struct {
    char dummy;
} NullObj;

typedef struct {
    void* value;
} ReturnObj;

typedef struct {
    char message[1024];    
} ErrorObj;

typedef struct sObject {
    bool marked; // para el GC
    struct sObject* next; // el siguiente objeto
    ObjectType type;
    void* value;
} Object;

// environment
// Package
typedef struct {
    unsigned hashCode;
    Object* value;
} Package;

// HashTable
typedef struct {
    int count;
    int capacity;
    Package** items;
} HashTable;

typedef struct _Environment {
    HashTable* store;
    struct _Environment* outer;
} Environment;
// environment

typedef struct {
    IdentifierNode* parameters[255];
    ArrayStmt* body;
    int arity;
    Environment* env;
} FunctionObj;

// Array de argumentos evaluados.
typedef struct {
    Object* arguments[255];
} Arguments;

/*================================================================/
* PUBLIC OBJECT API
*=================================================================*/
void freeObject(Object* obj);
char* inspect(Object* obj);

// environment API
Environment* newEnvironment();
Environment* newEnclosedEnvironment(Environment* outer);
Object* get(Environment* env, char* name);
Object* set(Environment* env, char* name, Object* value);
// environment
#endif