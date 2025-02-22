#include <stdlib.h>
#include "ast.h"

void ast_func_append(ast_func **head, ast_func *func) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = func;
}

ast_func *ast_func_create(char *name, ast_var *args, ast_type *type, ast_stmt *code) {
    ast_func *ret = malloc(sizeof(ast_func));
    ret->name = name;
    ret->args = args;
    ret->type = type;
    ret->code = code;
    ret->next = NULL;
    return ret;
}

void ast_struct_append(ast_struct **head, ast_struct *strct) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = strct;
}

ast_struct *ast_struct_create(char *name, ast_var *fields) {
    ast_struct *ret = malloc(sizeof(ast_struct));
    ret->name = name;
    ret->fields = fields;
    ret->next = NULL;
    return ret;
}

void ast_impl_append(ast_impl **head, ast_impl *impl) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = impl;
}

ast_impl *ast_impl_create(char *name, ast_func *funcs) {
    ast_impl *ret = malloc(sizeof(ast_impl));
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

ast_var *ast_var_create(char *name, ast_type *type, ast_expr *value) {
    ast_var *ret = malloc(sizeof(ast_var));
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

ast_type *ast_type_create(char *name) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->vnt = AST_TYPE_BASE;
    ret->name = name;
    ret->next = NULL;
}

ast_type *ast_type_make_ref(ast_type *subtype) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->vnt = AST_TYPE_REF;
    ret->subtype = subtype;
    ret->next = NULL;
}

ast_type *ast_type_make_arr(ast_type *subtype, ast_expr *arrlen) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->vnt = AST_TYPE_ARR;
    ret->arrlen = arrlen;
    ret->subtype = subtype;
    ret->next = NULL;
}

ast_type *ast_type_make_slice(ast_type *subtype) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->vnt = AST_TYPE_SLICE;
    ret->subtype = subtype;
    ret->next = NULL;
}

ast_type *ast_type_make_tuple(ast_type *subtypes) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->vnt = AST_TYPE_TUPLE;
    ret->subtype = subtypes;
    ret->next = NULL;
}

void ast_expr_append(ast_expr **head, ast_expr *expr) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = expr;
}

ast_expr *ast_expr_create(ast_expr_vnt vnt, char *data) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->vnt = vnt;
    ret->data = data;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_tuple(ast_expr *exprs) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->vnt = AST_EXPR_TUPLE;
    ret->rhs = exprs;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_dot(ast_expr *expr, char *data) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->vnt = AST_EXPR_DOT;
    ret->lhs = expr;
    ret->data = data;
    ret->next = NULL;
    return ret;
}

ast_expr *ast_expr_make_op(ast_expr_vnt vnt, ast_expr *lhs, ast_expr *rhs) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->vnt = vnt;
    ret->lhs = lhs;
    ret->rhs = rhs;
    ret->next = NULL;
    return ret;
}
