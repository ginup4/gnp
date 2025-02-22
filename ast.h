#ifndef AST_H
#define AST_H

// typedefs

typedef struct ast_func {
    char *name;
    struct ast_var *args;
    struct ast_type *type;
    struct ast_stmt *body;
    struct ast_func *next;
} ast_func;

typedef struct ast_struct {
    char *name;
    struct ast_var *fields;
    struct ast_struct *next;
} ast_struct;

typedef struct ast_impl {
    char *name;
    struct ast_func *funcs;
    struct ast_impl *next;
} ast_impl;

typedef struct ast_var {
    char *name;
    struct ast_type *type;
    struct ast_expr *value;
    struct ast_var *next;
} ast_var;

typedef enum ast_type_vnt {
    AST_TYPE_BASE,
    AST_TYPE_REF,
    AST_TYPE_ARR,
    AST_TYPE_SLICE,
    AST_TYPE_TUPLE,
} ast_type_vnt;

typedef struct ast_type {
    enum ast_type_vnt vnt;
    char *name;
    struct ast_type *subtype;
    struct ast_expr *arrlen;
    struct ast_type *next;
} ast_type;

typedef enum ast_stmt_vnt {
    AST_STMT_VAR,
    AST_STMT_EXPR,
    AST_STMT_RETURN,
    AST_STMT_BREAK,
    AST_STMT_CONTINUE,
    AST_STMT_LOOP,
    AST_STMT_WHILE,
    AST_STMT_IF,
    AST_STMT_ELSE,
} ast_stmt_vnt;

typedef struct ast_stmt {
    enum ast_stmt_vnt vnt;
    struct ast_var *var;
    struct ast_expr *expr;
    struct ast_stmt *body;
    struct ast_stmt *els;
    struct ast_stmt *next;
} ast_stmt;

typedef enum ast_expr_vnt {
    AST_EXPR_IDENT,
    AST_EXPR_NUM_LIT,
    AST_EXPR_STR_LIT,
    AST_EXPR_CHAR_LIT,
    AST_EXPR_TRUE,
    AST_EXPR_FALSE,
    AST_EXPR_NULL,
    AST_EXPR_TUPLE,
    AST_EXPR_DOT,
    AST_OP_CALL,
    AST_OP_INDEX,
    AST_OP_ASGN,
    AST_OP_ADD_ASGN,
    AST_OP_SUB_ASGN,
    AST_OP_MUL_ASGN,
    AST_OP_DIV_ASGN,
    AST_OP_MOD_ASGN,
    AST_OP_OR_ASGN,
    AST_OP_AND_ASGN,
    AST_OP_XOR_ASGN,
    AST_OP_COMP_EQ,
    AST_OP_COMP_NE,
    AST_OP_COMP_LE,
    AST_OP_COMP_GE,
    AST_OP_COMP_LT,
    AST_OP_COMP_GT,
    AST_OP_LOG_OR,
    AST_OP_LOG_AND,
    AST_OP_LOG_NOT,
    AST_OP_BIT_OR,
    AST_OP_BIT_AND,
    AST_OP_BIT_XOR,
    AST_OP_BIT_NOT,
    AST_OP_ADD,
    AST_OP_SUB,
    AST_OP_MUL,
    AST_OP_DIV,
    AST_OP_MOD,
    AST_OP_NEG,
    AST_OP_REF,
    AST_OP_DEREF,
} ast_expr_vnt;

typedef struct ast_expr {
    enum ast_expr_vnt vnt;
    char *data;
    struct ast_expr *lhs;
    struct ast_expr *rhs;
    struct ast_expr *next;
} ast_expr;

// globals

extern ast_func *glob_funcs;
extern ast_struct *glob_structs;
extern ast_impl *glob_impls;
extern ast_var *glob_vars;

// functions

void ast_func_append(ast_func **, ast_func *);
ast_func *ast_func_create(char *, ast_var *, ast_type *, ast_stmt *);

void ast_struct_append(ast_struct **, ast_struct *);
ast_struct *ast_struct_create(char *, ast_var *);

void ast_impl_append(ast_impl **, ast_impl *);
ast_impl *ast_impl_create(char *, ast_func *);

void ast_var_append(ast_var **, ast_var *);
ast_var *ast_var_create(char *, ast_type *, ast_expr *);

void ast_type_append(ast_type **, ast_type *);
ast_type *ast_type_create(char *);
ast_type *ast_type_make_ref(ast_type *);
ast_type *ast_type_make_arr(ast_type *, ast_expr *);
ast_type *ast_type_make_slice(ast_type *);
ast_type *ast_type_make_tuple(ast_type *);

void ast_stmt_append(ast_stmt **, ast_stmt *);
ast_stmt *ast_stmt_create(ast_stmt_vnt, ast_var *, ast_expr *, ast_stmt *, ast_stmt *);

void ast_expr_append(ast_expr **, ast_expr *);
ast_expr *ast_expr_create(ast_expr_vnt, char *);
ast_expr *ast_expr_make_tuple(ast_expr *);
ast_expr *ast_expr_make_dot(ast_expr *, char *);
ast_expr *ast_expr_make_op(ast_expr_vnt, ast_expr *, ast_expr *);

#endif
