#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "generator.h"

void generate_func(FILE *file, ast_func *func) {
    fprintf(file, "gnp_%s:\n", func->name);
    fprintf(file, "\tpush %%rbp\n");
    fprintf(file, "\tmov %%rsp,%%rbp\n");
    // actual code
    fprintf(file, "\tleave\n");
    fprintf(file, "\tret\n\n");
}

void generate_var(FILE *file, ast_var *var) {
    fprintf(file, "\tgnp_%s:\n", var->name);
    fprintf(file, "# init value for %s\n\n", var->name);
}

void generate(FILE *file, ast_prog *prog) {
    ast_var *var = prog->vars;
    if(var) {
        fprintf(file, "\t.data\n");
    }
    while(var) {
        generate_var(file, var);
        var = var->next;
    }

    ast_func *func = prog->funcs;
    if(func) {
        fprintf(file, "\t.text\n");
        fprintf(file, "\t.globl gnp_main\n");
    }
    while(func) {
        generate_func(file, func);
        func = func->next;
    }
}
