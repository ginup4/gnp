#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ast.h"
#include "lines.h"

#include "error.h"

extern int yyparse();
extern FILE *yyin;

void usage(char *progname) {
    fprintf(stderr, "Usage: %s [INPUT]", progname);
    exit(EXIT_FAILURE);
}

#include "print_ast.c"

int main(int argc, char **argv) {
    if(argc > 1) {
        yyin = fopen(argv[1], "r");
        if(!yyin) {
            perror(argv[1]);
            return EXIT_FAILURE;
        }
        filename = argv[1];
    } else {
        yyin = stdin;
        filename = "<stdin>";
    }

    if(yyparse()) {
        return EXIT_FAILURE;
    }

    ast_func *func = prog.funcs;
    while(func) {
        print_func(func);
        printf("\n");
        func = func->next;
    }
    ast_struct *strct = prog.structs;
    while(strct) {
        print_struct(strct);
        printf("\n");
        strct = strct->next;
    }
    ast_impl *impl = prog.impls;
    while(impl) {
        print_impl(impl);
        printf("\n");
        impl = impl->next;
    }
    ast_var *var = prog.vars;
    while(var) {
        printf("var ");
        print_var(var);
        printf("\n");
        var = var->next;
    }

    return EXIT_SUCCESS;
}
