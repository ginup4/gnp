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

void ast_func_free(ast_func *func) {
    free(func->name);
    ast_var *var = func->args;
    while(var) {
        ast_var_free(var);
        var = var->next;
    }
    if(func->type) {
        ast_type_free(func->type);
    }
    ast_stmts_free(func->body);
    free(func);
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

void ast_struct_free(ast_struct *strct) {
    free(strct->name);
    ast_var *var = strct->fields;
    while(var) {
        ast_var_free(var);
        var = var->next;
    }
    ast_func *func = strct->funcs;
    while(func) {
        ast_func_free(func);
        func = func->next;
    }
    free(strct);
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

void ast_impl_free(ast_impl *impl) {
    free(impl->name);
    ast_func *func = impl->funcs;
    while(func) {
        ast_func_free(func);
        func = func->next;
    }
    free(impl);
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

void ast_var_free(ast_var *var) {
    free(var->name);
    if(var->type) {
        ast_type_free(var->type);
    }
    if(var->expr) {
        ast_expr_free(var->expr);
    }
    free(var);
}

ast_var *ast_var_create(YYLTYPE loc, char *name, ast_type *type, ast_expr *expr) {
    ast_var *ret = malloc(sizeof(ast_var));
    ret->loc = loc;
    ret->name = name;
    ret->type = type;
    ret->expr = expr;
    ret->next = NULL;
    return ret;
}

void ast_type_append(ast_type **head, ast_type *type) {
    while(*head) {
        head = &(*head)->next;
    }
    *head = type;
}

void ast_type_free(ast_type *type) {
    ast_type *subtype;
    switch(type->vnt) {
    case AST_TYPE_BASE:
        if(type->pointed_vnt == AST_SYMBOL_UNRESOLVED) {
            free(type->pointed.name);
        }
        break;
    case AST_TYPE_REF:
    case AST_TYPE_ARR:
    case AST_TYPE_SLICE:
        ast_type_free(type->subtype);
        break;
    case AST_TYPE_TUPLE:
        subtype = type->subtype;
        while(subtype) {
            ast_type_free(subtype);
            subtype = subtype->next;
        }
        break;
    }
    free(type);
}

ast_type *ast_type_create(YYLTYPE loc, char *name) {
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = loc;
    ret->vnt = AST_TYPE_BASE;
    ret->pointed_vnt = AST_SYMBOL_UNRESOLVED;
    ret->pointed.name = name;
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

void ast_stmts_free(ast_stmt *stmt) {
    ast_stmt *temp;
    ast_stmt *els;
    while(stmt) {
        switch(stmt->vnt) {
        case AST_STMT_VAR:
            ast_var_free(stmt->var);
            break;
        case AST_STMT_EXPR:
        case AST_STMT_FREE:
            ast_expr_free(stmt->expr);
            break;
        case AST_STMT_RETURN:
            if(stmt->expr) {
                ast_expr_free(stmt->expr);
            }
            break;
        case AST_STMT_LOOP:
            ast_stmts_free(stmt->body);
            break;
        case AST_STMT_WHILE:
            ast_expr_free(stmt->expr);
            ast_stmts_free(stmt->body);
            break;
        case AST_STMT_IF:
            ast_expr_free(stmt->expr);
            ast_stmts_free(stmt->body);
            els = stmt->els;
            while(els && els->expr) {
                ast_expr_free(els->expr);
                ast_stmts_free(els->body);
                els = els->els;
            }
            if(els) {
                ast_stmts_free(els->body);
            }
            break;
        default:
            break;
        }
        temp = stmt->next;
        free(stmt);
        stmt = temp;
    }
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

void ast_expr_free(ast_expr *expr) {
    ast_expr *subexpr;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        if(expr->pointed_vnt == AST_SYMBOL_UNRESOLVED) {
            free(expr->pointed.data);
        }
        break;
    case AST_EXPR_NUM_LIT:
    case AST_EXPR_STR_LIT:
    case AST_EXPR_CHAR_LIT:
    case AST_EXPR_TRUE:
    case AST_EXPR_FALSE:
    case AST_EXPR_NULL:
        free(expr->pointed.data);
        break;
    case AST_EXPR_TUPLE:
        subexpr = expr->rhs;
        while(subexpr) {
            ast_expr_free(subexpr);
            subexpr = subexpr->next;
        }
        break;
    case AST_EXPR_DOT:
        ast_expr_free(expr->lhs);
        if(expr->pointed_vnt == AST_SYMBOL_UNRESOLVED) {
            free(expr->pointed.data);
        }
        break;
    case AST_OP_CALL:
        ast_expr_free(expr->lhs);
        subexpr = expr->rhs;
        while(subexpr) {
            ast_expr_free(subexpr);
            subexpr = subexpr->next;
        }
        break;
    case AST_OP_LOG_NOT:
    case AST_OP_BIT_NOT:
    case AST_OP_NEG:
    case AST_OP_REF:
    case AST_OP_DEREF:
    case AST_OP_ALLOC:
    case AST_OP_PUT:
    case AST_OP_TAKE:
        ast_expr_free(expr->rhs);
        break;
    case AST_OP_INC:
    case AST_OP_DEC:
        ast_expr_free(expr->lhs);
        break;
    default:
        ast_expr_free(expr->lhs);
        ast_expr_free(expr->rhs);
        break;
    }
    free(expr);
}

ast_expr *ast_expr_create(YYLTYPE loc, ast_expr_vnt vnt, char *data) {
    ast_expr *ret = malloc(sizeof(ast_expr));
    ret->loc = loc;
    ret->vnt = vnt;
    ret->pointed_vnt = AST_SYMBOL_UNRESOLVED;
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
    ret->pointed_vnt = AST_SYMBOL_UNRESOLVED;
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
