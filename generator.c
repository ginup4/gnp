#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "generator.h"

void generate_func(FILE *file, ast_func *func) {
    fprintf(file, "gnp_%s:\n", func->name);
    fprintf(file, "    push %%rbp\n");
    fprintf(file, "    mov %%rsp,%%rbp\n");
    // actual code
    fprintf(file, "    leave\n");
    fprintf(file, "    ret\n\n");
}

void generate_var(FILE *file, ast_var *var) {
    fprintf(file, "gnp_%s:\n", var->name);
    fprintf(file, "    # init value for %s\n\n", var->name);
}

void generate(FILE *file, ast_prog *prog) {
    printf("codegen lol\n");
    return;
    ast_var *var = prog->vars;
    if(var) {
        fprintf(file, "    .data\n");
    }
    while(var) {
        generate_var(file, var);
        var = var->next;
    }

    ast_func *func = prog->funcs;
    if(func) {
        fprintf(file, "    .text\n");
        fprintf(file, "    .globl gnp_main\n");
    }
    while(func) {
        generate_func(file, func);
        func = func->next;
    }
}
