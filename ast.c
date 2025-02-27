#include <stdlib.h>
#include <string.h>
#include "ast.h"

ast_prog glob_program = {NULL, NULL, NULL, NULL, NULL};

void ast_func_append(ast_func **head, ast_func *func) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = func;
}

ast_func *ast_func_create(YYLTYPE loc, char *name, ast_var *args, ast_type *type, ast_stmt *body) {
    ast_func *ret = malloc(sizeof(ast_func));
    ret->loc = loc;
    ret->name = name;
    ret->args = args;
    ret->type = type;
    ret->body = body;
    ret->next = NULL;
    return ret;
}

void ast_struct_append(ast_struct **head, ast_struct *strct) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = strct;
}

ast_struct *ast_struct_create(YYLTYPE loc, char *name, ast_var *fields) {
    ast_struct *ret = malloc(sizeof(ast_struct));
    ret->loc = loc;
    ret->name = name;
    ret->fields = fields;
    ret->funcs = NULL;
    ret->next = NULL;
    return ret;
}

void ast_impl_append(ast_impl **head, ast_impl *impl) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = impl;
}

ast_impl *ast_impl_create(YYLTYPE loc, char *name, ast_func *funcs) {
    ast_impl *ret = malloc(sizeof(ast_impl));
    ret->loc = loc;
    ret->name = name;
    ret->funcs = funcs;
    ret->next = NULL;
    return ret;
}

void ast_var_append(ast_var **head, ast_var *var) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = var;
}

ast_var *ast_var_create(YYLTYPE loc, char *name, ast_type *type, ast_expr *value) {
    ast_var *ret = malloc(sizeof(ast_var));
    ret->loc = loc;
    ret->name = name;
    ret->type = type;
    ret->value = value;
    ret->next = NULL;
    return ret;
}

void ast_type_append(ast_type **head, ast_type *type) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = type;
}

ast_type *ast_type_create(YYLTYPE loc, char *name) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_BASE;
    ret->name = name;
    ret->next = NULL;
    return ret;
}

ast_type *ast_type_make_ref(YYLTYPE loc, ast_type *subtype) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_REF;
    ret->subtype = subtype;
    ret->next = NULL;
    return ret;
}

ast_type *ast_type_make_arr(YYLTYPE loc, ast_type *subtype, ast_expr *arrlen) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_ARR;
    ret->arrlen = arrlen;
    ret->subtype = subtype;
    ret->next = NULL;
    return ret;
}

ast_type *ast_type_make_slice(YYLTYPE loc, ast_type *subtype) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_SLICE;
    ret->subtype = subtype;
    ret->next = NULL;
    return ret;
}

ast_type *ast_type_make_tuple(YYLTYPE loc, ast_type *subtypes) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_TUPLE;
    ret->subtype = subtypes;
    ret->next = NULL;
    return ret;
}

void ast_stmt_append(ast_stmt **head, ast_stmt *stmt) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = stmt;
}

ast_stmt *ast_stmt_create(YYLTYPE loc, ast_stmt_vnt vnt, ast_var *var, ast_expr *expr, ast_stmt *body, ast_stmt *els) {
    ast_stmt *ret = malloc(sizeof(ast_stmt));
    ret->loc = loc;
    ret->vnt = vnt;
    ret->var = var;
    ret->expr = expr;
    ret->body = body;
    ret->els = els;
    ret->next = NULL;
    return ret;
}

void ast_expr_append(ast_expr **head, ast_expr *expr) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = expr;
}

ast_expr *ast_expr_create(YYLTYPE loc, ast_expr_vnt vnt, char *data) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->loc = loc;
    ret->vnt = vnt;
    ret->pointed.data = data;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_tuple(YYLTYPE loc, ast_expr *exprs) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->loc = loc;
    ret->vnt = AST_EXPR_TUPLE;
    ret->rhs = exprs;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_dot(YYLTYPE loc, ast_expr *expr, char *data) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->loc = loc;
    ret->vnt = AST_EXPR_DOT;
    ret->lhs = expr;
    ret->pointed.data = data;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_op(YYLTYPE loc, ast_expr_vnt vnt, ast_expr *lhs, ast_expr *rhs) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->loc = loc;
    ret->vnt = vnt;
    ret->lhs = lhs;
    ret->rhs = rhs;
    ret->next = NULL;
    return ret;
}

ast_symbol *ast_symbol_push(ast_prog *prog, YYLTYPE loc, char *name, ast_symbol_vnt vnt, void *pointed, int scope) {
    ast_symbol *symbol = prog->symbols;
    while(symbol) {
        if(symbol->scope < scope) {
            break;
        }
        if(strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    symbol = malloc(sizeof(ast_symbol));
    symbol->loc = loc;
    symbol->name = strdup(name);
    symbol->vnt = vnt;
    switch(vnt) {
    case AST_SYMBOL_STRUCT:
        symbol->pointed.strct = pointed;
        break;
    case AST_SYMBOL_FUNC:
        symbol->pointed.func = pointed;
        break;
    case AST_SYMBOL_VAR:
        symbol->pointed.var = pointed;
        break;
    }
    symbol->scope = scope;
    symbol->next = prog->symbols;
    prog->symbols = symbol;
    return NULL;
}

ast_symbol *ast_symbol_find(ast_prog *prog, char *name) {
    ast_symbol *symbol = prog->symbols;
    while(symbol) {
        if(strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

void ast_symbol_pop(ast_prog *prog, int scope) {
    ast_symbol *symbol;
    while(prog->symbols) {
        if(prog->symbols->scope < scope) {
            break;
        }
        symbol = prog->symbols;
        prog->symbols = symbol->next;
        free(symbol->name);
        free(symbol);
    }
}
