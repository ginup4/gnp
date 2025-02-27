#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "analyzer.h"
#include "ast.h"
#include "error.h"

void generate_symbols(ast_prog *prog) {
    ast_symbol *old_symbol;
    ast_struct *strct = prog->structs;
    while(strct) {
        old_symbol = ast_symbol_push(prog, strct->loc, strct->name, AST_SYMBOL_STRUCT, strct, 0);
        if(old_symbol) {
            log_error("symbol already defined", strct->loc);
            log_note("here", old_symbol->loc);
        }
        strct = strct->next;
    }
    ast_func *func = prog->funcs;
    while(func) {
        old_symbol = ast_symbol_push(prog, func->loc, func->name, AST_SYMBOL_FUNC, func, 0);
        if(old_symbol) {
            log_error("symbol already defined", func->loc);
            log_note("here", old_symbol->loc);
        }
        func = func->next;
    }
    ast_var *var = prog->vars;
    while(var) {
        old_symbol = ast_symbol_push(prog, var->loc, var->name, AST_SYMBOL_VAR, var, 0);
        if(old_symbol) {
            log_error("symbol already defined", var->loc);
            log_note("here", old_symbol->loc);
        }
        var = var->next;
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

void resolve_symbols_expr(ast_prog *prog, ast_expr *expr) {
    ast_expr *subexpr;
    ast_symbol *symbol;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        symbol = ast_symbol_find(prog, expr->pointed.data);
        if(symbol) {
            switch(symbol->vnt) {
            case AST_SYMBOL_FUNC:
                expr->pointed_vnt = AST_SYMBOL_FUNC;
                expr->pointed.func = symbol->pointed.func;
                break;
            case AST_SYMBOL_STRUCT:
                expr->pointed_vnt = AST_SYMBOL_STRUCT;
                expr->pointed.strct = symbol->pointed.strct;
                break;
            case AST_SYMBOL_VAR:
                expr->pointed_vnt = AST_SYMBOL_VAR;
                expr->pointed.var = symbol->pointed.var;
                break;
            }
        } else {
            log_error("unknown symbol", expr->loc);
        }
        break;
    case AST_EXPR_NUM_LIT:
    case AST_EXPR_STR_LIT:
    case AST_EXPR_CHAR_LIT:
    case AST_EXPR_TRUE:
    case AST_EXPR_FALSE:
    case AST_EXPR_NULL:
        break;
    case AST_EXPR_TUPLE:
        subexpr = expr->rhs;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            subexpr = subexpr->next;
        }
        break;
    case AST_EXPR_DOT:
        resolve_symbols_expr(prog, expr->lhs);
        break;
    case AST_OP_CALL:
        resolve_symbols_expr(prog, expr->lhs);
        subexpr = expr->rhs;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            subexpr = subexpr->next;
        }
        break;
    case AST_OP_INDEX:
        resolve_symbols_expr(prog, expr->lhs);
        resolve_symbols_expr(prog, expr->rhs);
        break;
    case AST_OP_LOG_NOT:
    case AST_OP_BIT_NOT:
    case AST_OP_NEG:
    case AST_OP_REF:
    case AST_OP_DEREF:
    case AST_OP_ALLOC:
    case AST_OP_PUT:
    case AST_OP_TAKE:
        resolve_symbols_expr(prog, expr->rhs);
        break;
    case AST_OP_INC:
    case AST_OP_DEC:
        resolve_symbols_expr(prog, expr->lhs);
        break;
    default:
        resolve_symbols_expr(prog, expr->lhs);
        resolve_symbols_expr(prog, expr->rhs);
        break;
    }
}

void resolve_symbols_stmts(ast_prog *prog, ast_stmt *stmt, int scope) {
    ast_stmt *els;
    ast_symbol *old_symbol;
    while(stmt) {
        switch(stmt->vnt) {
        case AST_STMT_BREAK:
        case AST_STMT_CONTINUE:
            break;
        case AST_STMT_VAR:
        case AST_STMT_RETURN:
            if(stmt->expr) {
                resolve_symbols_expr(prog, stmt->expr);
            }
            break;
        case AST_STMT_EXPR:
        case AST_STMT_FREE:
            resolve_symbols_expr(prog, stmt->expr);
            break;
        case AST_STMT_LOOP:
            resolve_symbols_stmts(prog, stmt->body, scope + 1);
            break;
        case AST_STMT_WHILE:
            resolve_symbols_expr(prog, stmt->expr);
            resolve_symbols_stmts(prog, stmt->body, scope + 1);
            break;
        case AST_STMT_IF:
            resolve_symbols_expr(prog, stmt->expr);
            resolve_symbols_stmts(prog, stmt->body, scope + 1);
            els = stmt->els;
            while(els && els->expr) {
                resolve_symbols_expr(prog, els->expr);
                resolve_symbols_stmts(prog, els->body, scope + 1);
                els = els->els;
            }
            if(els) {
                resolve_symbols_stmts(prog, els->body, scope + 1);
            }
            break;
        }
        if(stmt->vnt == AST_STMT_VAR) {
            old_symbol = ast_symbol_push(prog, stmt->var->loc, stmt->var->name, AST_SYMBOL_VAR, stmt->var, scope);
            if(old_symbol) {
                log_error("variable already defined", stmt->var->loc);
                log_note("here", old_symbol->loc);
            }
        }
        stmt = stmt->next;
    }
    ast_symbol_pop(prog, scope);
}

void resolve_symbols_func(ast_prog *prog, ast_func *func) {
    ast_symbol *old_symbol;
    ast_var *arg = func->args;
    while(arg) {
        old_symbol = ast_symbol_push(prog, arg->loc, arg->name, AST_SYMBOL_VAR, arg, 2);
        if(old_symbol) {
            log_error("argument already defined", arg->loc);
            log_note("here", old_symbol->loc);
        }
        arg = arg->next;
    }
    resolve_symbols_stmts(prog, func->body, 2);
}

void resolve_symbols(ast_prog *prog) {
    ast_struct *strct = prog->structs;
    ast_func *func;
    while(strct) {
        ast_symbol_push(prog, strct->loc, "Self", AST_SYMBOL_STRUCT, strct, 1);
        func = strct->funcs;
        while(func) {
            resolve_symbols_func(prog, func);
            func = func->next;
        }
        ast_symbol_pop(prog, 1);
        strct = strct->next;
    }
    func = prog->funcs;
    while(func) {
        resolve_symbols_func(prog, func);
        func = func->next;
    }
}

void analyze_ast(ast_prog *prog) {
    generate_symbols(prog);
    if(errors) return;
    attach_impls(prog);
    deduplicate_struct_fields(prog);
    if(errors) return;
    resolve_symbols(prog);
    ast_symbol_pop(prog, 0);
    if(errors) return;
}
