#include "interpreter.h"

static void repl();
static void test();
static char* readFile(const char* path);
static void runFile(const char* path);

int main(int argc, const char* argv[]) {
    initEvaluator();

    if (argc == 1) {
        // test();
        repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: cmonk [path]\n");
        exit(74);
    }

    freeEvaluator();
    return 0;
}

static void repl() {
	fprintf(stdout, "Hello! This is the Monkey programming language!\n");	
	fprintf(stdout, "Feel free to type in commands\n");
    char line[1024];
    for (;;) {
        fprintf(stdout, "> ");
        if (!fgets(line, sizeof(line), stdin)) {
            fprintf(stdout, "\n");
            break;
        }
        if (strcmp(line, "quit\n") == 0) break;
        interpret(line);
    }
}

static void test() {
    interpret("let fib = fn(n) { if(n < 2) {return n;} else {return fib(n-1) + fib(n-2);}; }; fib(27);");
    // initLexer("1 + 2 * 3 / 4;");
    // Token tok;
    // for (tok = nextToken(); tok.type != T_EOF; tok = nextToken()) {
    //     fprintf(stdout, "Token(%s, '%s');\n", tokenNames[tok.type], extractLiteral(tok.position));
    // }
    // fprintf(stdout, "\n");
}

static char* readFile(const char* path) {
    return NULL;
}

static void runFile(const char* path) {
    return;
}