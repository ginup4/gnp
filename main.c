#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ast.h"

extern int yyparse();
extern FILE *yyin;

void usage(char *progname) {
    fprintf(stderr, "Usage: %s [INPUT]", progname);
    exit(EXIT_FAILURE);
}

ast_func *glob_funcs = NULL;
ast_struct *glob_structs = NULL;
ast_impl *glob_impls = NULL;
ast_var *glob_vars = NULL;

#include "print_ast.c"

int main(int argc, char **argv) {
    if(argc > 1) {
        yyin = fopen(argv[1], "r");
        if(!yyin) {
            perror(argv[1]);
            return EXIT_FAILURE;
        }
    } else {
        yyin = stdin;
    }
    if(yyparse()) {
        return EXIT_FAILURE;
    }

    ast_func *func = glob_funcs;
    while(func) {
        print_func(func);
        printf("\n");
        func = func->next;
    }
    ast_struct *strct = glob_structs;
    while(strct) {
        print_struct(strct);
        printf("\n");
        strct = strct->next;
    }
    ast_impl *impl = glob_impls;
    while(impl) {
        print_impl(impl);
        printf("\n");
        impl = impl->next;
    }
    ast_var *var = glob_vars;
    while(var) {
        printf("var ");
        print_var(var);
        printf("\n");
        var = var->next;
    }

    return EXIT_SUCCESS;
}
