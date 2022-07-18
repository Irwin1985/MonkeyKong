#include "interpreter.h"

Object* TrueObj;
Object* FalseObj;
Object* NilObj;

// Creamos el primer objeto en la lista enlazada de objetos.
static Object* firstObject;
static int numObjects; // número de objetos creados actualmente (malloc)
static int maxObjects; // número máximo de objetos para lanzar el GC.
// Environment global
static Environment* globalEnv;

/*================================================================/
* Forwarded declarations.
*=================================================================*/
static void mark(Object* object);
static void markAll();
static void markEnvironment(Environment* env);
static void sweep();
void gc();
static Object* newObject(ObjectType type, void* value);
static Object* newBoolean(bool value);
static Object* newNull();
Object* interpret(const char* source);
void initEvaluator();
void freeEvaluator();
static Object* newInteger(int value);
static Object* newString(char* value);
static Object* newReturn(Object* value);
static Object* newError(char* message);
static Object* newFunction(FunctionNode* node, Environment* env);
static Object* evalBangOperatorExpression(Object* obj);
static Object* nativeBoolToBooleanObject(bool value);
static Object* evalMinusPrefixOperatorExpression(Object* obj);
static Object* evalPrefixExpression(TokenType ope, Object* right);
static bool inlist(const char* src, int argc, ...);
static Object* evalIntegerInfixExpression(TokenType ope, Object* left, Object* right);
static Object* evalStringInfixExpression(TokenType ope, Object* left, Object* right);
static Object* evalInfixExpression(TokenType ope, Object* left, Object* right);
static Arguments* evalArguments(CallNode* node, Environment* env);
static Object* applyFunction(Object* function, Arguments* args);
static Environment* extendFunctionEnv(FunctionObj* funObj, Arguments* args);
static Object* unwrapReturnValue(Object* evaluated);
static bool isTruthy(Object* object);
static bool isError(Object* object);
Object* evalIfExpression(IfNode* node, Environment* env);
Object* evalIdentifier(IdentifierNode* node,Environment* env);
Object* evalExpression(Expression* exp, Environment* env);
Object* evalBlockStatements(ArrayStmt* stmts, Environment* env);
Object* evalStatements(Statement* stmt, Environment* env);
Object* evalProgram(ArrayStmt* program, Environment* env);

/*================================================================/
* Implementation
*=================================================================*/
static void mark(Object* object) {
   if (object->marked) return;
   object->marked = true;
   if (object->type == FUNCTION_OBJ) {
      markEnvironment(((FunctionObj*)object->value)->env);
   }
}

static void markEnvironment(Environment* env) {
    for (int i = 0; i < env->store->count; i++) {
        mark(env->store->items[i]->value);
    }
    if (env->outer != NULL) {
        markEnvironment(env->outer);
    }
}

static void markAll() {
    mark(TrueObj);
    mark(FalseObj);
    mark(NilObj);
    // marcar el environment global
    markEnvironment(globalEnv);
}

static void sweep() {
    Object** object = &firstObject;
    while (*object) {
        if (!(*object)->marked) {
            // Este objeto es inalcalzable así que lo liberamos.
            Object* unreached = *object;
            *object = unreached->next; // desvincular el objeto
            freeObject(unreached); // libera el objeto envuelto y a sí mismo.            
            numObjects -= 1;
        } else {
            // Este objeto sigue activo así que lo marcamos como false para 
            // chequearlo en la siguiente ronda.
            (*object)->marked = false;
            object = &(*object)->next;
        }
    }
}

void gc() {
    int curNumObjects = numObjects;
    markAll(); // marcamos todos los objetos activos en este punto.
    sweep(); // todos los que no fueron marcados serán eliminados.
    // maxObjects = (numObjects == 0) ? GC_MAX_OBJECTS : numObjects * 2;

    fprintf(stdout, "Collected %d objects, %d remaining.\n", curNumObjects - numObjects, (numObjects-3));
}

/*================================================================/
* Funciones factory.
*=================================================================*/
static Object* newObject(ObjectType type, void* value) {
    if (numObjects == maxObjects) {
        gc();
    }
    Object* object = createObject(Object);
    object->type   = type;
    object->next   = firstObject; // apunta al último objeto creado.
    firstObject    = object; // ahora el último objeto creado es este 'object'.
    object->marked = false;
    object->value  = value;

    numObjects += 1; // otro objeto ha sido creado así que incrementamos el número.    

    return object;
}

static Object* newBoolean(bool value) {
    BooleanObj* bo = createObject(BooleanObj);
    bo->value = value;

    return newObject(BOOLEAN_OBJ, bo);
}

static Object* newNull() {
    NullObj* ni_obj = createObject(NullObj);
    return newObject(NULL_OBJ, ni_obj);
}

static Object* newError(char* message) {
    ErrorObj* error = createObject(ErrorObj);
    strcpy_s(error->message, 1024, message);

    return newObject(ERROR_OBJ, error);
}

static Object* newFunction(FunctionNode* node, Environment* env) {
    FunctionObj* func = createObject(FunctionObj);

    memcpy(func->parameters, node->parameters, sizeof(func->parameters));
    func->arity = node->arity;
    func->body = node->body;
    func->env = env;

    return newObject(FUNCTION_OBJ, func);
}

/*================================================================/
* Inicializador del evaluador.
*=================================================================*/
Object* interpret(const char* source) {
    initLexer(source);
    ArrayStmt* program = parseProgram();
    if (program != NULL) {
        Object* evaluated = evalProgram(program, globalEnv);
		if (evaluated != NULL) {
			fprintf(stdout, "%s\n", inspect(evaluated));
		}
        // gc();
        freeProgram(program);
    }
}

void initEvaluator() {
    // ********************************* //
    firstObject = NULL; // el objeto raíz siempre es NULL.
    numObjects = 0;
    maxObjects = GC_MAX_OBJECTS;
    // ********************************* //
    globalEnv = newEnvironment();
    TrueObj  = newBoolean(true);
    FalseObj = newBoolean(false);
    NilObj   = newNull();
}

void freeEvaluator() {
    // liberar los objetos estáticos.
    // freeObject(TrueObj);
    // numObjects -= 1;
    // freeObject(FalseObj);
    // numObjects -= 1;
    // freeObject(NilObj);
    // numObjects -= 1;
    int curNumObjects = numObjects;
    sweep(); // eliminar todo sin dejar nada
    fprintf(stdout, "Collected %d objects, %d remaining.\n", curNumObjects - numObjects, numObjects);
}

/********************************************************
* Helper functions
*********************************************************/
static Object* newInteger(int value) {
    IntegerObj* intObj = createObject(IntegerObj);
    intObj->value = value;

    return newObject(INTEGER_OBJ, intObj);
}

static Object* newString(char* value) {
    StringObj* strObj = createObject(StringObj);
    strObj->value = strdup(value);

    return newObject(STRING_OBJ, strObj);
}

static Object* newReturn(Object* value) {
    ReturnObj* retObj = createObject(ReturnObj);
    retObj->value = value;

    return newObject(RETURN_OBJ, retObj);
}

static Object* evalBangOperatorExpression(Object* obj) {
    switch (obj->type) {
    case BOOLEAN_OBJ:
        return (obj == TrueObj) ? FalseObj : TrueObj; // !true | !false
    case NULL_OBJ:
        return TrueObj;
    default:
        return FalseObj;
    }
}

static Object* nativeBoolToBooleanObject(bool value) {
    return value ? TrueObj : FalseObj;
}

static Object* evalMinusPrefixOperatorExpression(Object* obj) {
    if (obj->type != INTEGER_OBJ) {
        return newError("Operand must be an integer type.");
    }
    IntegerObj* integer = (IntegerObj*)obj->value;

    return newInteger(integer->value * -1);
}

static Object* evalPrefixExpression(TokenType ope, Object* right) {
    switch (ope) {
    case T_BANG:
        return evalBangOperatorExpression(right);
    case T_MINUS:
        return evalMinusPrefixOperatorExpression(right);
    default:
        return NilObj;
    }
}

static Object* evalIntegerInfixExpression(TokenType ope, Object* left, Object* right) {    
    int leftVal = ((IntegerObj*)left->value)->value;
    int rightVal = ((IntegerObj*)right->value)->value;

    switch (ope) {
        case T_PLUS:
            return newInteger(leftVal + rightVal);
        case T_MINUS:
            return newInteger(leftVal - rightVal);
        case T_ASTERISK:
            return newInteger(leftVal * rightVal);
        case T_SLASH: 
            if (rightVal == 0) {
                fprintf(stdout, "Division by zero.\n");
                exit(74);
            }
            return newInteger(leftVal / rightVal);
        case T_LT:
            return nativeBoolToBooleanObject(leftVal < rightVal);
        case T_GT:
            return nativeBoolToBooleanObject(leftVal > rightVal);
        case T_EQ:
            return nativeBoolToBooleanObject(leftVal == rightVal);
        case T_NOT_EQ:
            return nativeBoolToBooleanObject(leftVal != rightVal);
        default:
            return newError("Unknown operator for integer operands.");
    }
}

static Object* evalStringInfixExpression(TokenType ope, Object* left, Object* right) {
    switch (ope) {
        case T_PLUS: {
            StringObj* leftStr = (StringObj*)left->value;
            StringObj* rightStr = (StringObj*)right->value;
            int len = strlen(leftStr->value) + strlen(rightStr->value);
            char *str = (char*)malloc(len + 1);
            if (str == NULL) {
                fprintf(stderr, "ERROR: out of memory.\n");
                exit(74);
            }
            sprintf_s(str, len+1, "%s%s", leftStr->value, rightStr->value);

            return newString(str);
        }
        default:
            return NilObj;
    }
}

static Object* evalInfixExpression(TokenType ope, Object* left, Object* right) {
    if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ)
        return evalIntegerInfixExpression(ope, left, right);
    
    if (left->type == STRING_OBJ && right->type == STRING_OBJ)
        return evalStringInfixExpression(ope, left, right);

    if (left->type != right->type) {
        return newError("type mismatch. Operators must have the same type.");
    }

    switch (ope) {
        case T_EQ:
            return nativeBoolToBooleanObject(left == right);
        case T_NOT_EQ:
            return nativeBoolToBooleanObject(left != right);
        default:
            return newError("Not supported operator.");
    }
}

static Arguments* evalArguments(CallNode* node, Environment* env) {
    Arguments* args = createObject(Arguments);

    Object* result;
    for (int i = 0; i < node->argc; i++) {
        result = evalExpression(node->arguments[i], env);
        if (isError(result)) {
            args->arguments[0] = result;
            return args;
        }
        args->arguments[i] = result;
    }

    return args;
}

static Object* applyFunction(Object* function, Arguments* args) {
    if (function->type != FUNCTION_OBJ) {
        return newError("not a function.");
    }
    FunctionObj* funObj = (FunctionObj*)function->value;

    Environment* extendedEnv = extendFunctionEnv(funObj, args);
    Object* evaluated = evalBlockStatements(funObj->body, extendedEnv);

    return unwrapReturnValue(evaluated);
}

static Environment* extendFunctionEnv(FunctionObj* funObj, Arguments* args) {
    Environment* env = newEnclosedEnvironment(funObj->env);
    for (int i = 0; i < funObj->arity; i++) {
        char* name = extractLiteral(funObj->parameters[i]->token.position);
        set(env, name, args->arguments[i]);        
        free(name);
    }
    return env;
}

static Object* unwrapReturnValue(Object* evaluated) {
    if (evaluated->type == RETURN_OBJ) {
        return ((ReturnObj*)evaluated->value)->value;
    }
    return evaluated;
}

static bool isTruthy(Object* object) {
    return (object == FalseObj || object == NilObj) ? false : true;
}

static bool isError(Object* object) {
    if (object != NULL) {
        return (object->type == ERROR_OBJ) ? true : false;
    }
    return false;
}

Object* evalIfExpression(IfNode* node, Environment* env) {
    Object* condition = evalExpression(node->condition, env);
    if (isError(condition)) {
        return condition;
    }
    if (isTruthy(condition)) {
        return evalBlockStatements(node->consequence, env);
    }
    if (node->alternative != NULL) {
        return evalBlockStatements(node->alternative, env);
    }
    return NilObj;
}

Object* evalIdentifier(IdentifierNode* node,Environment* env) {
    char* literal = extractLiteral(node->token.position);
    Object* val = get(env, literal);
    if (val == NULL) {
        char msg[1024];
        sprintf_s(msg, strlen(msg), "identifier not found: %s.", literal);
        return newError(msg);
    }
    free(literal);
    return val;
}

/**************************************************************************
* Evaluador de expresiones
***************************************************************************/
Object* evalExpression(Expression* exp, Environment* env) {
    switch (exp->type) {
    case NT_INTEGER: {
        char* literal = extractLiteral(((IntegerNode*)exp->node)->token.position);
        int value = atoi(literal);
        free(literal);
        return newInteger(value);
    }
    case NT_STRING:
        char* literal = extractLiteral(((StringNode*)exp->node)->token.position);
        return newString(literal);
    case NT_NULL:
        return NilObj;
    case NT_BOOLEAN:
        return nativeBoolToBooleanObject(((BooleanNode*)exp->node)->value);        
    case NT_PREFIX:
        {
            PrefixNode* prefix = (PrefixNode*)exp->node;
            Object* right = evalExpression(prefix->right, env);
            if (isError(right)) {
                return right;
            }
            return evalPrefixExpression(prefix->operator, right);
        }
    case NT_INFIX:
        {
            InfixNode* infix = (InfixNode*)exp->node;            
            Object* left = evalExpression(infix->left, env);
            if (isError(left)) {
                return left;
            }

            Object* right = evalExpression(infix->right, env);
            if (isError(right)) {
                return right;
            }

            return evalInfixExpression(infix->operator, left, right);
        }
    case NT_IF:
        return evalIfExpression((IfNode*)exp->node, env);
    case NT_FUNCTION:
        return newFunction((FunctionNode*)exp->node, env);
    case NT_IDENT:
        return evalIdentifier(((IdentifierNode*)exp->node), env);
    case NT_CALL:
        Object* function = evalExpression(((CallNode*)exp->node)->function, env);
        if (isError(function)) return function;
        Arguments* args = evalArguments((CallNode*)exp->node, env);
        if (isError(args->arguments[0])) return args->arguments[0];

        return applyFunction(function, args);
    default:
        return NilObj;
    }
}

Object* evalBlockStatements(ArrayStmt* stmts, Environment* env) {
    Object* result;
    for (int i = 0; i < stmts->count; i++) {
        result = evalStatements(stmts->statements[i], env);

        if (result->type == RETURN_OBJ || result->type == ERROR_OBJ) {
            return result;
        }
    }
    return result;
}

Object* evalStatements(Statement* stmt, Environment* env) {
    switch (stmt->type) {
    case NT_LET: {
        Object* val = evalExpression(((LetStatement*)stmt->node)->value, env);
        if (isError(val)) return val;
        char* literal = extractLiteral(((LetStatement*)stmt->node)->name->token.position);
        Object* res = set(env, literal, val);
        free(literal);
        return res;
    }    
    case NT_RETURN: {
        Object* val = evalExpression(((ReturnStatement*)stmt->node)->value, env);
        if (isError(val)) return val;
        return newReturn(val);
    }
    case NT_EXPR:
        return evalExpression(((ExpressionStatement*)stmt->node)->expression, env);
    default:
        return NilObj;
    }
}

Object* evalProgram(ArrayStmt* program, Environment* env) {
    Object* result = NilObj;
    for (int i = 0; i < program->count; i++) {
        result = evalStatements(program->statements[i], env);
        switch (result->type) {
            case RETURN_OBJ:
                return ((ReturnObj*)result->value)->value;
            case ERROR_OBJ:
                return result;
        }
    }
    return result;
}