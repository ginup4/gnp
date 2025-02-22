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

void print_type(ast_type *type) {
    switch(type->vnt) {
    case AST_TYPE_BASE:
        printf("%s", type->name);
        break;
    case AST_TYPE_REF:
        printf("&");
        print_type(type->subtype);
        break;
    case AST_TYPE_ARR:
        printf("[");
        print_type(type->subtype);
        printf(";n]");
        break;
    case AST_TYPE_SLICE:
        printf("[");
        print_type(type->subtype);
        printf("]");
        break;
    case AST_TYPE_TUPLE:
        printf("(");
        ast_type *subtype = type->subtype;
        while(subtype) {
            print_type(subtype);
            printf(",");
            subtype = subtype->next;
        }
        printf(")");
        break;
    }
}

void print_expr(ast_expr *expr) {
    printf("(");
    ast_expr *subexpr;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        printf("%s", expr->data);
        break;
    case AST_EXPR_NUM_LIT:
        printf("%s", expr->data);
        break;
    case AST_EXPR_STR_LIT:
        printf("%s", expr->data);
        break;
    case AST_EXPR_CHAR_LIT:
        printf("%s", expr->data);
        break;
    case AST_EXPR_TRUE:
        printf("true");
        break;
    case AST_EXPR_FALSE:
        printf("false");
        break;
    case AST_EXPR_NULL:
        printf("null");
        break;
    case AST_EXPR_TUPLE:
        subexpr = expr->rhs;
        while(subexpr) {
            print_expr(subexpr);
            printf(",");
            subexpr = subexpr->next;
        }
        break;
    case AST_EXPR_DOT:
        print_expr(expr->lhs);
        printf(".%s", expr->data);
        break;
    case AST_OP_CALL:
        print_expr(expr->lhs);
        subexpr = expr->rhs;
        printf("(");
        while(subexpr) {
            print_expr(subexpr);
            printf(",");
            subexpr = subexpr->next;
        }
        printf(")");
        break;
    case AST_OP_INDEX:
        print_expr(expr->lhs);
        printf("[");
        print_expr(expr->rhs);
        printf("]");
        break;
    case AST_OP_ASGN:
        print_expr(expr->lhs);
        printf("=");
        print_expr(expr->rhs);
        break;
    case AST_OP_ADD_ASGN:
        print_expr(expr->lhs);
        printf("+=");
        print_expr(expr->rhs);
        break;
    case AST_OP_SUB_ASGN:
        print_expr(expr->lhs);
        printf("-=");
        print_expr(expr->rhs);
        break;
    case AST_OP_MUL_ASGN:
        print_expr(expr->lhs);
        printf("*=");
        print_expr(expr->rhs);
        break;
    case AST_OP_DIV_ASGN:
        print_expr(expr->lhs);
        printf("/=");
        print_expr(expr->rhs);
        break;
    case AST_OP_MOD_ASGN:
        print_expr(expr->lhs);
        printf("%=");
        print_expr(expr->rhs);
        break;
    case AST_OP_OR_ASGN:
        print_expr(expr->lhs);
        printf("|=");
        print_expr(expr->rhs);
        break;
    case AST_OP_AND_ASGN:
        print_expr(expr->lhs);
        printf("$=");
        print_expr(expr->rhs);
        break;
    case AST_OP_XOR_ASGN:
        print_expr(expr->lhs);
        printf("^=");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_EQ:
        print_expr(expr->lhs);
        printf("==");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_NE:
        print_expr(expr->lhs);
        printf("!=");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_LE:
        print_expr(expr->lhs);
        printf("<=");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_GE:
        print_expr(expr->lhs);
        printf(">=");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_LT:
        print_expr(expr->lhs);
        printf("<");
        print_expr(expr->rhs);
        break;
    case AST_OP_COMP_GT:
        print_expr(expr->lhs);
        printf(">");
        print_expr(expr->rhs);
        break;
    case AST_OP_LOG_OR:
        print_expr(expr->lhs);
        printf("||");
        print_expr(expr->rhs);
        break;
    case AST_OP_LOG_AND:
        print_expr(expr->lhs);
        printf("$$");
        print_expr(expr->rhs);
        break;
    case AST_OP_LOG_NOT:
        printf("!");
        print_expr(expr->rhs);
        break;
    case AST_OP_BIT_OR:
        print_expr(expr->lhs);
        printf("|");
        print_expr(expr->rhs);
        break;
    case AST_OP_BIT_AND:
        print_expr(expr->lhs);
        printf("$");
        print_expr(expr->rhs);
        break;
    case AST_OP_BIT_XOR:
        print_expr(expr->lhs);
        printf("^");
        print_expr(expr->rhs);
        break;
    case AST_OP_BIT_NOT:
        printf("~");
        print_expr(expr->rhs);
        break;
    case AST_OP_ADD:
        print_expr(expr->lhs);
        printf("+");
        print_expr(expr->rhs);
        break;
    case AST_OP_SUB:
        print_expr(expr->lhs);
        printf("-");
        print_expr(expr->rhs);
        break;
    case AST_OP_MUL:
        print_expr(expr->lhs);
        printf("*");
        print_expr(expr->rhs);
        break;
    case AST_OP_DIV:
        print_expr(expr->lhs);
        printf("/");
        print_expr(expr->rhs);
        break;
    case AST_OP_MOD:
        print_expr(expr->lhs);
        printf("%");
        print_expr(expr->rhs);
        break;
    case AST_OP_NEG:
        printf("-");
        print_expr(expr->rhs);
        break;
    case AST_OP_REF:
        printf("&");
        print_expr(expr->rhs);
        break;
    case AST_OP_DEREF:
        printf("@");
        print_expr(expr->rhs);
        break;
    }
    printf(")");
}

void print_var(ast_var *var) {
    printf("'%s' : ", var->name);
    if(var->type) {
        print_type(var->type);
    } else {
        printf("?");
    }
    if(var->value) {
        printf(" = ");
        print_expr(var->value);
    }
}

void print_func(ast_func *func) {
    printf("fn '%s' ( ", func->name);
    ast_var *arg = func->args;
    while(arg) {
        print_var(arg);
        printf(" , ");
        arg = arg->next;
    }
    printf(" ) : ");
    if(func->type) {
        print_type(func->type);
    } else {
        printf("?");
    }
    printf("\n");
}

void print_struct(ast_struct *strct) {
    printf("struct '%s' { ", strct->name);
    ast_var *field = strct->fields;
    while(field) {
        print_var(field);
        printf(" , ");
        field = field->next;
    }
    printf(" }\n");
}

void print_impl(ast_impl *impl) {
    printf("impl '%s' {\n", impl->name);
    ast_func *func = impl->funcs;
    while(func) {
        printf("\t");
        print_func(func);
        func = func->next;
    }
    printf("}\n");
}

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
        func = func->next;
    }
    ast_struct *strct = glob_structs;
    while(strct) {
        print_struct(strct);
        strct = strct->next;
    }
    ast_impl *impl = glob_impls;
    while(impl) {
        print_impl(impl);
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
