#include <stdlib.h>
#include <string.h>
#include "analyzer.h"
#include "ast.h"
#include "error.h"

#include <stdio.h>

void generate_symbols(ast_prog *prog) {
    ast_symbol *new_symbol;
    ast_symbol **last_symbol = &prog->symbols;
    ast_struct *strct = prog->structs;
    while(strct) {
        new_symbol = malloc(sizeof(ast_symbol));
        new_symbol->loc = strct->loc;
        new_symbol->name = strdup(strct->name);
        new_symbol->vnt = AST_SYMBOL_STRUCT;
        new_symbol->pointed.strct = strct;
        new_symbol->next = NULL;
        *last_symbol = new_symbol;
        last_symbol = &new_symbol->next;
        strct = strct->next;
    }
    ast_func *func = prog->funcs;
    while(func) {
        new_symbol = malloc(sizeof(ast_symbol));
        new_symbol->loc = func->loc;
        new_symbol->name = strdup(func->name);
        new_symbol->vnt = AST_SYMBOL_FUNC;
        new_symbol->pointed.func = func;
        new_symbol->next = NULL;
        *last_symbol = new_symbol;
        last_symbol = &new_symbol->next;
        func = func->next;
    }
    ast_var *var = prog->vars;
    while(var) {
        new_symbol = malloc(sizeof(ast_symbol));
        new_symbol->loc = var->loc;
        new_symbol->name = strdup(var->name);
        new_symbol->vnt = AST_SYMBOL_VAR;
        new_symbol->pointed.var = var;
        new_symbol->next = NULL;
        *last_symbol = new_symbol;
        last_symbol = &new_symbol->next;
        var = var->next;
    }
    ast_symbol *symbol1 = prog->symbols;
    ast_symbol *symbol2;
    while(symbol1) {
        symbol2 = symbol1->next;
        while(symbol2) {
            if(strcmp(symbol1->name, symbol2->name) == 0) {
                log_error("symbol already defined", symbol2->loc);
                log_note("here", symbol1->loc);
            }
            symbol2 = symbol2->next;
        }
        symbol1 = symbol1->next;
    }
}

void attach_impls(ast_prog *prog) {
    ast_struct *strct;
    ast_impl *impl;
    ast_func *func;
    ast_func **strct_func;
    while(prog->impls) {
        impl = prog->impls;
        prog->impls = impl->next;
        impl->next = NULL;
        strct = prog->structs;
        while(strct) {
            if(strcmp(strct->name, impl->name) == 0) {
                break;
            }
            strct = strct->next;
        }
        if(!strct) {
            log_error("impl for undefined struct", impl->loc);
            continue;
        }
        while(impl->funcs) {
            func = impl->funcs;
            impl->funcs = func->next;
            func->next = NULL;
            strct_func = &strct->funcs;
            while(*strct_func) {
                if(strcmp(func->name, (*strct_func)->name) == 0) {
                    log_error("method already defined", func->loc);
                    log_note("here", (*strct_func)->loc);
                    break;
                }
                strct_func = &(*strct_func)->next;
            }
            if(!(*strct_func)) {
                *strct_func = func;
            }
        }
        free(impl->name);
        free(impl);
    }
}

void deduplicate_struct_fields(ast_prog *prog) {
    ast_struct *strct = prog->structs;
    ast_var *field1;
    ast_var *field2;
    while(strct) {
        field1 = strct->fields;
        while(field1) {
            field2 = field1->next;
            while(field2) {
                if(strcmp(field1->name, field2->name) == 0) {
                    log_error("field already defined", field2->loc);
                    log_note("here", field1->loc);
                }
                field2 = field2->next;
            }
            field1 = field1->next;
        }
        strct = strct->next;
    }
}

void analyze_ast(ast_prog *prog) {
    generate_symbols(prog);
    if(errors) return;
    attach_impls(prog);
    deduplicate_struct_fields(prog);
    if(errors) return;
}
