#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "type_checker.h"
#include "analyzer.h"
#include "ast.h"
#include "error.h"

bool type_cmp(ast_type *a, ast_type *b) {
    if(a->vnt != b->vnt) {
        return false;
    }
    switch(a->vnt)
    {
    case AST_TYPE_BASE:
        return a->pointed.strct == b->pointed.strct;
    case AST_TYPE_ARR:
        panic("array not implemented");
        return false;
    case AST_TYPE_REF:
    case AST_TYPE_SLICE:
        return type_cmp(a->subtype, b->subtype);
    case AST_TYPE_TUPLE:
        a = a->subtype;
        b = b->subtype;
        while(a && b) {
            if(!type_cmp(a, b)) {
                return false;
            }
            a = a->next;
            b = b->next;
        }
        if(a || b) {
            return false;
        }
        return true;
    }
    panic("unreachable");
    return false;
}

ast_type *type_copy(ast_type *type) {
    ast_type **subret, *subtype;
    ast_type *ret = malloc(sizeof(ast_type));
    ret->loc = type->loc;
    ret->vnt = type->vnt;
    switch(type->vnt) {
    case AST_TYPE_BASE:
        ret->pointed_vnt = type->pointed_vnt;
        switch(type->pointed_vnt)
        {
        case AST_SYMBOL_STRUCT:
            ret->pointed.strct = type->pointed.strct;
            break;
        default:
            printf("pointed vnt: %d\n", type->pointed_vnt);
            panic("copying unresolved type");
        }
        break;
    case AST_TYPE_REF:
    case AST_TYPE_SLICE:
        ret->subtype = type_copy(type->subtype);
        break;
    case AST_TYPE_ARR:
        panic("copying array type not implemented");
        break;
    case AST_TYPE_TUPLE:
        subtype = type->subtype;
        subret = &ret->subtype;
        while(subtype) {
            *subret = type_copy(subtype);
            subret = &(*subret)->next;
            subtype = subtype->next;
        }
        break;
    }
    ret->next = NULL;
    return ret;
}

void type_check_expr(ast_expr *expr) {
    //ast_expr *subexpr;
    ast_type *type;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        switch(expr->pointed_vnt) {
        case AST_SYMBOL_VAR:
            expr->type = type_copy(expr->pointed.var->type);
            break;
        default:
            break; // func and struct dont have type
        }
        break;
    case AST_EXPR_NUM_LIT:
    case AST_EXPR_STR_LIT:
    case AST_EXPR_CHAR_LIT:
    case AST_EXPR_TRUE:
    case AST_EXPR_FALSE:
    case AST_EXPR_NULL:
        break; // type already set
    case AST_EXPR_DOT:
        panic("dot not implemented");
        break;
    case AST_EXPR_TUPLE:
        panic("tuple not implemented");
        break;
    case AST_OP_CALL:
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt == AST_SYMBOL_STRUCT) {
            expr->type = ast_type_make_base(default_loc, expr->lhs->pointed.strct);
        } else if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt == AST_SYMBOL_FUNC) {
            expr->type = type_copy(expr->lhs->pointed.func->type);
        } else if(expr->lhs->vnt == AST_EXPR_DOT) {
            panic("method call not implemented");
        } else {
            panic("unreachable");
        }
        break;
    case AST_OP_INDEX:
        type_check_expr(expr->lhs);
        type_check_expr(expr->rhs);
        type = expr->lhs->type;
        if(type->vnt == AST_TYPE_REF) {
            type = type->subtype;
        }
        if(type->vnt == AST_TYPE_SLICE || type->vnt == AST_TYPE_ARR) {
            expr->type = type_copy(type->subtype);
        } else {
            log_error("cannot index this type", expr->lhs->loc);
            exit(EXIT_FAILURE);
        }
        if(expr->rhs->type->vnt != AST_TYPE_BASE || !expr->rhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        break;
    case AST_OP_ASGN:
        type_check_expr(expr->lhs);
        // TODO only if assignable
        type_check_expr(expr->rhs);
        if(!type_cmp(expr->lhs->type, expr->rhs->type)) {
            log_error("mismatched types", expr->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = type_copy(expr->rhs->type);
        break;
    case AST_OP_ADD_ASGN:
    case AST_OP_SUB_ASGN:
    case AST_OP_MUL_ASGN:
    case AST_OP_DIV_ASGN:
    case AST_OP_MOD_ASGN:
    case AST_OP_OR_ASGN:
    case AST_OP_AND_ASGN:
    case AST_OP_XOR_ASGN:
        type_check_expr(expr->lhs);
        // TODO only if assignable
        type_check_expr(expr->rhs);
        if(!type_cmp(expr->lhs->type, expr->rhs->type)) {
            log_error("mismatched types", expr->loc);
            exit(EXIT_FAILURE);
        }
        if(expr->rhs->type->vnt != AST_TYPE_BASE || !expr->rhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = type_copy(expr->rhs->type);
        break;
        break;
    case AST_OP_COMP_EQ:
    case AST_OP_COMP_NE:
        type_check_expr(expr->lhs);
        type_check_expr(expr->rhs);
        if(!type_cmp(expr->lhs->type, expr->rhs->type)) {
            log_error("mismatched types", expr->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        break;
    case AST_OP_COMP_LE:
    case AST_OP_COMP_GE:
    case AST_OP_COMP_LT:
    case AST_OP_COMP_GT:
        type_check_expr(expr->lhs);
        type_check_expr(expr->rhs);
        if(!type_cmp(expr->lhs->type, expr->rhs->type)) {
            log_error("mismatched types", expr->loc);
            exit(EXIT_FAILURE);
        }
        if(expr->rhs->type->vnt != AST_TYPE_BASE || !expr->rhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        break;
    case AST_OP_LOG_OR:
    case AST_OP_LOG_AND:
        type_check_expr(expr->lhs);
        type_check_expr(expr->rhs);
        if(expr->rhs->type->vnt != AST_TYPE_BASE || expr->rhs->type->pointed.strct != glob_struct_bool) {
            log_error("not a bool", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        if(expr->lhs->type->vnt != AST_TYPE_BASE || expr->lhs->type->pointed.strct != glob_struct_bool) {
            log_error("not a bool", expr->lhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        break;
    case AST_OP_LOG_NOT:
        type_check_expr(expr->rhs);
        if(expr->rhs->type->vnt != AST_TYPE_BASE || expr->rhs->type->pointed.strct != glob_struct_bool) {
            log_error("not a bool", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        break;
    case AST_OP_BIT_OR:
    case AST_OP_BIT_AND:
    case AST_OP_BIT_XOR:
    case AST_OP_ADD:
    case AST_OP_SUB:
    case AST_OP_MUL:
    case AST_OP_DIV:
    case AST_OP_MOD:
        type_check_expr(expr->lhs);
        type_check_expr(expr->rhs);
        if(!type_cmp(expr->lhs->type, expr->rhs->type)) {
            log_error("mismatched types", expr->loc);
            exit(EXIT_FAILURE);
        }
        if(expr->rhs->type->vnt != AST_TYPE_BASE || !expr->rhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = type_copy(expr->lhs->type);
        break;
    case AST_OP_NEG:
        type_check_expr(expr->rhs);
        if(expr->rhs->type->vnt != AST_TYPE_BASE || !expr->rhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = type_copy(expr->rhs->type);
        break;
    case AST_OP_REF:
        type_check_expr(expr->rhs);
        // TODO only if assignable
        expr->type = ast_type_make_ref(default_loc, type_copy(expr->rhs->type));
        break;
    case AST_OP_DEREF:
        type_check_expr(expr->rhs);
        if(expr->rhs->type->vnt == AST_TYPE_REF) {
            expr->type = type_copy(expr->rhs->type->subtype);
        } else {
            log_error("not a ref", expr->rhs->loc);
            exit(EXIT_FAILURE);
        }
        break;
    case AST_OP_INC:
    case AST_OP_DEC:
        type_check_expr(expr->lhs);
        // TODO only if assignable
        if(expr->lhs->type->vnt != AST_TYPE_BASE || !expr->lhs->type->pointed.strct->numeric) {
            log_error("nonnumeric type", expr->lhs->loc);
            exit(EXIT_FAILURE);
        }
        expr->type = type_copy(expr->lhs->type);
        break;
    default:
        panic("other expr, not implemented");
    }
}

void type_check_var(ast_var *var) {
    if(var->expr) {
        type_check_expr(var->expr);
        if(var->type) {
            if(!type_cmp(var->type, var->expr->type)) {
                log_error("variable initialized with mismatched type", var->expr->loc);
            }
        } else {
            var->type = type_copy(var->expr->type);
        }
    }
}

void type_check_stmts(ast_type *functype, ast_stmt *stmt) {
    ast_stmt *els;
    while(stmt) {
        switch(stmt->vnt) {
        case AST_STMT_BREAK:
        case AST_STMT_CONTINUE:
            break;
        case AST_STMT_VAR:
            type_check_var(stmt->var);
            break;
        case AST_STMT_RETURN:
            if(stmt->expr) {
                type_check_expr(stmt->expr);
                if(!type_cmp(stmt->expr->type, functype)) {
                    log_error("mismatched return type", stmt->expr->loc);
                }
            } else {
                if(functype->vnt != AST_TYPE_BASE || functype->pointed.strct != glob_struct_void) {
                    log_error("function needs a return value", stmt->loc);
                }
            }
            break;
        case AST_STMT_EXPR:
            type_check_expr(stmt->expr);
            break;
        case AST_STMT_FREE:
            type_check_expr(stmt->expr);
            // TODO only ref to slice
            break;
        case AST_STMT_LOOP:
            type_check_stmts(functype, stmt->body);
            break;
        case AST_STMT_WHILE:
            type_check_expr(stmt->expr);
            if(stmt->expr->type->vnt != AST_TYPE_BASE || stmt->expr->type->pointed.strct != glob_struct_bool) {
                log_error("not a bool", stmt->expr->loc);
            }
            type_check_stmts(functype, stmt->body);
            break;
        case AST_STMT_IF:
            type_check_expr(stmt->expr);
            if(stmt->expr->type->vnt != AST_TYPE_BASE || stmt->expr->type->pointed.strct != glob_struct_bool) {
                log_error("not a bool", stmt->expr->loc);
            }
            type_check_stmts(functype, stmt->body);
            els = stmt->els;
            while(els && els->expr) {
                type_check_expr(els->expr);
                if(els->expr->type->vnt != AST_TYPE_BASE || els->expr->type->pointed.strct != glob_struct_bool) {
                    log_error("not a bool", els->expr->loc);
                }
                type_check_stmts(functype, els->body);
                els = els->els;
            }
            if(els) {
                type_check_stmts(functype, els->body);
            }
            break;
        }
        stmt = stmt->next;
    }
}

void type_check(ast_prog *prog) {
    ast_var *var = prog->vars;
    while(var) {
        type_check_var(var);
        var = var->next;
    }
    if(errors) return;
    ast_func *func = prog->funcs;
    while(func) {
        type_check_stmts(func->type, func->body);
        func = func->next;
    }
    if(errors) return;
}
