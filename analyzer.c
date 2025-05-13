#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "analyzer.h"
#include "type_checker.h"
#include "ast.h"
#include "error.h"

// temp
#include <stdio.h>

struct location default_loc = {0};

void generate_base_types(ast_prog *prog) {
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "void", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "char", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "bool", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "isize", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "usize", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "i8", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "u8", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "i16", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "u16", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "i32", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "u32", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "i64", NULL));
    ast_struct_append(&prog->structs, ast_struct_create(default_loc, "u64", NULL));
}

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
            ast_impl_free(impl);
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
                    ast_func_free(func);
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
    ast_func *func;
    bool is_const;
    bool found;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        symbol = ast_symbol_find(prog, expr->pointed.data);
        if(symbol) {
            free(expr->pointed.data);
            switch(symbol->vnt) {
            case AST_SYMBOL_FUNC: // can only be called (for later)
                expr->pointed_vnt = AST_SYMBOL_FUNC;
                expr->pointed.func = symbol->pointed.func;
                expr->is_const = false;
                break;
            case AST_SYMBOL_STRUCT: // can only access fields (for later)
                expr->pointed_vnt = AST_SYMBOL_STRUCT;
                expr->pointed.strct = symbol->pointed.strct;
                expr->is_const = false; // temp ; Self(1, 2) -> true ; Self.dupa(1, 2) -> false
                break;
            case AST_SYMBOL_VAR:
                expr->pointed_vnt = AST_SYMBOL_VAR;
                expr->pointed.var = symbol->pointed.var;
                expr->is_const = false;
                break;
            case AST_SYMBOL_UNRESOLVED:
                if(errors == 0) {
                    panic("unresolved symbol in symbol stack");
                }
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
        expr->is_const = true;
        break;
    case AST_EXPR_TUPLE:
        subexpr = expr->rhs;
        is_const = true;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            is_const = is_const && subexpr->is_const;
            subexpr = subexpr->next;
        }
        expr->is_const = is_const;
        break;
    case AST_EXPR_DOT:
        resolve_symbols_expr(prog, expr->lhs);
        switch(expr->lhs->pointed_vnt) {
        case AST_SYMBOL_FUNC:
            log_error("functions have no members", expr->loc);
            break;
        case AST_SYMBOL_STRUCT:
            found = false;
            func = expr->lhs->pointed.strct->funcs;
            while(func) {
                if(strcmp(func->name, expr->pointed.data) == 0) {
                    free(expr->pointed.data);
                    expr->pointed_vnt = AST_SYMBOL_FUNC;
                    expr->pointed.func = func;
                    found = true;
                    break;
                }
                func = func->next;
            }
            if(!found) {
                log_error("associated function not found", expr->loc);
            }
            break;
        case AST_SYMBOL_VAR: // leave for type checker
            break;
        case AST_SYMBOL_UNRESOLVED:
            if(errors == 0) {
                panic("symbol resolved to unresolved symbol");
            }
            break;
        }
        expr->is_const = false; // temp ; if have type consts ; only if lhs IS struct
        break;
    case AST_OP_CALL:
        resolve_symbols_expr(prog, expr->lhs);
        subexpr = expr->rhs;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            subexpr = subexpr->next;
        }
        expr->is_const = false;
        break;
    case AST_OP_INDEX:
        resolve_symbols_expr(prog, expr->lhs);
        resolve_symbols_expr(prog, expr->rhs);
        expr->is_const = false;
        break;
    case AST_OP_LOG_NOT:
    case AST_OP_BIT_NOT:
    case AST_OP_NEG:
        resolve_symbols_expr(prog, expr->rhs);
        expr->is_const = expr->rhs->is_const;
        break;
    case AST_OP_REF:
    case AST_OP_DEREF:
    case AST_OP_ALLOC:
    case AST_OP_PUT:
    case AST_OP_TAKE:
        resolve_symbols_expr(prog, expr->rhs);
        expr->is_const = false;
        break;
    case AST_OP_INC:
    case AST_OP_DEC:
        resolve_symbols_expr(prog, expr->lhs);
        expr->is_const = false;
        break;
    case AST_OP_ASGN:
    case AST_OP_ADD_ASGN:
    case AST_OP_SUB_ASGN:
    case AST_OP_MUL_ASGN:
    case AST_OP_DIV_ASGN:
    case AST_OP_MOD_ASGN:
    case AST_OP_OR_ASGN:
    case AST_OP_AND_ASGN:
    case AST_OP_XOR_ASGN:
    case AST_OP_REALLOC:
        resolve_symbols_expr(prog, expr->lhs);
        resolve_symbols_expr(prog, expr->rhs);
        expr->is_const = false;
        break;
    default:
        resolve_symbols_expr(prog, expr->lhs);
        resolve_symbols_expr(prog, expr->rhs);
        expr->is_const = (expr->lhs->is_const && expr->rhs->is_const);
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
            if(stmt->var->expr) {
                resolve_symbols_expr(prog, stmt->var->expr);
            }
            break;
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
    ast_var *var = prog->vars;
    while(var) {
        if(var->expr) {
            resolve_symbols_expr(prog, var->expr);
        }
        var = var->next;
    }
}

void simplify_consts(ast_prog *prog) {
    // no simplification for now, just check
    ast_var *var = prog->vars;
    while(var) {
        if(var->expr && !var->expr->is_const) {
            log_error("value not known at compile time", var->expr->loc);
        }
        var = var->next;
    }
}

void resolve_types_type(ast_prog *prog, ast_type *type) {
    ast_symbol *symbol;
    ast_type *subtype;
    switch(type->vnt) {
    case AST_TYPE_BASE:
        symbol = ast_symbol_find(prog, type->pointed.name);
        if(symbol) {
            free(type->pointed.name);
            type->pointed.strct = symbol->pointed.strct;
        } else {
            log_error("unknown type", type->loc);
        }
        break;
    case AST_TYPE_REF:
    case AST_TYPE_ARR:
    case AST_TYPE_SLICE:
        resolve_types_type(prog, type->subtype);
        break;
    case AST_TYPE_TUPLE:
        subtype = type->subtype;
        while(subtype) {
            resolve_types_type(prog, subtype);
            subtype = subtype->next;
        }
        break;
    }
}

void resolve_types_var(ast_prog *prog, ast_var *var) {
    if(var->type) {
        resolve_types_type(prog, var->type);
    }
}

void resolve_types_stmts(ast_prog *prog, ast_stmt *stmt) {
    ast_stmt *els;
    while(stmt) {
        switch(stmt->vnt) {
        case AST_STMT_VAR:
            resolve_types_var(prog, stmt->var);
            break;
        case AST_STMT_LOOP:
            resolve_types_stmts(prog, stmt->body);
            break;
        case AST_STMT_WHILE:
            resolve_types_stmts(prog, stmt->body);
            break;
        case AST_STMT_IF:
            resolve_types_stmts(prog, stmt->body);
            els = stmt->els;
            while(els && els->expr) {
                resolve_types_stmts(prog, els->body);
                els = els->els;
            }
            if(els) {
                resolve_types_stmts(prog, els->body);
            }
            break;
        default:
            break;
        }
        stmt = stmt->next;
    }
}

void resolve_types_func(ast_prog *prog, ast_func *func) {
    ast_symbol *symbol;
    if(func->type) {
        resolve_types_type(prog, func->type);
    } else {
        func->type = ast_type_create(func->loc, NULL);
        symbol = ast_symbol_find(prog, "void");
        func->type->pointed.strct = symbol->pointed.strct;
    }
    ast_var *var = func->args;
    while(var) {
        resolve_types_var(prog, var);
        var = var->next;
    }
    resolve_types_stmts(prog, func->body);
}

void resolve_types(ast_prog *prog) {
    ast_struct *strct = prog->structs;
    ast_func *func;
    ast_var *var;
    while(strct) {
        ast_symbol_push(prog, strct->loc, "Self", AST_SYMBOL_STRUCT, strct, 1);
        var = strct->fields;
        while(var) {
            resolve_types_var(prog, var);
            var = var->next;
        }
        func = strct->funcs;
        while(func) {
            resolve_types_func(prog, func);
            func = func->next;
        }
        ast_symbol_pop(prog, 1);
        strct = strct->next;
    }
    func = prog->funcs;
    while(func) {
        resolve_types_func(prog, func);
        func = func->next;
    }
    var = prog->vars;
    while(var) {
        resolve_types_var(prog, var);
        var = var->next;
    }
}

void analyze_ast(ast_prog *prog) {
    generate_base_types(prog);
    generate_symbols(prog);
    if(errors) return;
    attach_impls(prog);
    deduplicate_struct_fields(prog);
    if(errors) return;
    resolve_types(prog);
    if(errors) return;
    resolve_symbols(prog);
    ast_symbol_pop(prog, 0);
    if(errors) return;
    type_check(prog);
    if(errors) return;
    simplify_consts(prog);
    if(errors) return;
}
