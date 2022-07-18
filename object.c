#include "object.h"

void freeObject(Object* obj) {
    if (obj->type == STRING_OBJ)
        free(((StringObj*)obj->value)->value);

    free(obj->value);
    free(obj);
}

// para imprimir los objetos
char* inspect(Object* obj) {
    char* out = (char*)malloc(sizeof(char) * 1024);
    if (out == NULL) {
        fprintf(stderr, "ERROR: not enough memory.\n");
        exit(74);
    }
    switch (obj->type) {
    case INTEGER_OBJ:
        sprintf_s(out, 1024, "%i", ((IntegerObj*)obj->value)->value);
        break;
    case STRING_OBJ: 
        sprintf_s(out, 1024, "%s", ((StringObj*)obj->value)->value);
        break;
    case BOOLEAN_OBJ:
        sprintf_s(out, 1024, "%s", (((BooleanObj*)obj->value)->value == true) ? "true" : "false");
        break;
    case NULL_OBJ:
        sprintf_s(out, 1024, "%s", "null");
        break;
    case RETURN_OBJ:
        {
            ReturnObj* retObj = ((ReturnObj*)obj->value);
            if (retObj->value != NULL) {
                sprintf_s(out, 1024, "%s", inspect(((ReturnObj*)obj->value)->value));
            }
            break;
        }
    case ERROR_OBJ:
        sprintf_s(out, 1024, "RUNTIME ERROR: %s", ((ErrorObj*)obj->value)->message);
        break;
    }
    return out;
}

// environment
// función hash original del libro K&R
unsigned hash(char *s){
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

static HashTable* newHashTable() {
    HashTable* ht = createObject(HashTable);
    ht->capacity = 0;
    ht->count = 0;
    ht->items = NULL;

    return ht;
}

static void setKey(HashTable* ht, char* key, Object* obj) {
    if (ht->capacity < (ht->count + 1)) {
        ht->capacity = (ht->capacity == 0) ? 8 : ht->capacity * 2; // factor de 2
        ht->items = realloc(ht->items, sizeof(Package) * ht->capacity);
    }
    // crear el paquete
    Package* pkg = createObject(Package);
    pkg->hashCode = hash(key);
    pkg->value = obj;

    // guardamos el paquete
    ht->items[ht->count] = pkg;
    ht->count += 1;
}

Object* getValue(HashTable* ht, char* key) {
    unsigned hashCode = hash(key);
    for (int i = 0; i < ht->count; i++) {
        if (((Package*)ht->items[i])->hashCode == hashCode) {
            return ht->items[i]->value;
        }
    }
    return NULL;
}

Environment* newEnvironment() {
    Environment* env = createObject(Environment);
    env->store = newHashTable();
    env->outer = NULL;

    return env;
}

Environment* newEnclosedEnvironment(Environment* outer) {
    Environment* env = newEnvironment();
    env->outer = outer;
    return env;
}

Object* get(Environment* env, char* name) {
    Object* obj = getValue(env->store, name);
    if (obj == NULL && env->outer != NULL) {        
        return get(env->outer, name);
    }
    return obj;
}

Object* set(Environment* env, char* name, Object* value) {
    setKey(env->store, name, value);
    return value;
}
// environment