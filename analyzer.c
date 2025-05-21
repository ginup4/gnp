#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "analyzer.h"
#include "type_checker.h"
#include "ast.h"
#include "error.h"

// TODO remove
#include <stdio.h>

ast_struct *glob_struct_void;
ast_struct *glob_struct_char;
ast_struct *glob_struct_bool;
ast_struct *glob_struct_isize;
ast_struct *glob_struct_usize;
ast_struct *glob_struct_i8;
ast_struct *glob_struct_u8;
ast_struct *glob_struct_i16;
ast_struct *glob_struct_u16;
ast_struct *glob_struct_i32;
ast_struct *glob_struct_u32;
ast_struct *glob_struct_i64;
ast_struct *glob_struct_u64;

void generate_base_types(ast_prog *prog) {
    glob_struct_void = ast_struct_create(default_loc, "void", NULL);
    glob_struct_void->size = 0;
    glob_struct_void->algn = 0;
    ast_struct_append(&prog->structs, glob_struct_void);
    glob_struct_char = ast_struct_create(default_loc, "char", NULL);
    glob_struct_char->size = 1;
    glob_struct_char->algn = 1;
    glob_struct_char->numeric = true;
    glob_struct_char->sign = false;
    ast_struct_append(&prog->structs, glob_struct_char);
    glob_struct_bool = ast_struct_create(default_loc, "bool", NULL);
    glob_struct_bool->size = 1;
    glob_struct_bool->algn = 1;
    ast_struct_append(&prog->structs, glob_struct_bool);
    glob_struct_isize = ast_struct_create(default_loc, "isize", NULL);
    glob_struct_isize->size = 8; // architecture specific
    glob_struct_isize->algn = 8;
    glob_struct_isize->numeric = true;
    glob_struct_isize->sign = true;
    ast_struct_append(&prog->structs, glob_struct_isize);
    glob_struct_usize = ast_struct_create(default_loc, "usize", NULL);
    glob_struct_usize->size = 8; // architecture specific
    glob_struct_usize->algn = 8;
    glob_struct_usize->numeric = true;
    glob_struct_usize->sign = false;
    ast_struct_append(&prog->structs, glob_struct_usize);
    glob_struct_i8 = ast_struct_create(default_loc, "i8", NULL);
    glob_struct_i8->size = 1;
    glob_struct_i8->algn = 1;
    glob_struct_i8->numeric = true;
    glob_struct_i8->sign = true;
    ast_struct_append(&prog->structs, glob_struct_i8);
    glob_struct_u8 = ast_struct_create(default_loc, "u8", NULL);
    glob_struct_u8->size = 1;
    glob_struct_u8->algn = 1;
    glob_struct_u8->numeric = true;
    glob_struct_u8->sign = false;
    ast_struct_append(&prog->structs, glob_struct_u8);
    glob_struct_i16 = ast_struct_create(default_loc, "i16", NULL);
    glob_struct_i16->size = 2;
    glob_struct_i16->algn = 2;
    glob_struct_i16->numeric = true;
    glob_struct_i16->sign = true;
    ast_struct_append(&prog->structs, glob_struct_i16);
    glob_struct_u16 = ast_struct_create(default_loc, "u16", NULL);
    glob_struct_u16->size = 2;
    glob_struct_u16->algn = 2;
    glob_struct_u16->numeric = true;
    glob_struct_u16->sign = false;
    ast_struct_append(&prog->structs, glob_struct_u16);
    glob_struct_i32 = ast_struct_create(default_loc, "i32", NULL);
    glob_struct_i32->size = 4;
    glob_struct_i32->algn = 4;
    glob_struct_i32->numeric = true;
    glob_struct_i32->sign = true;
    ast_struct_append(&prog->structs, glob_struct_i32);
    glob_struct_u32 = ast_struct_create(default_loc, "u32", NULL);
    glob_struct_u32->size = 4;
    glob_struct_u32->algn = 4;
    glob_struct_u32->numeric = true;
    glob_struct_u32->sign = false;
    ast_struct_append(&prog->structs, glob_struct_u32);
    glob_struct_i64 = ast_struct_create(default_loc, "i64", NULL);
    glob_struct_i64->size = 8;
    glob_struct_i64->algn = 8;
    glob_struct_i64->numeric = true;
    glob_struct_i64->sign = true;
    ast_struct_append(&prog->structs, glob_struct_i64);
    glob_struct_u64 = ast_struct_create(default_loc, "u64", NULL);
    glob_struct_u64->size = 8;
    glob_struct_u64->algn = 8;
    glob_struct_u64->numeric = true;
    glob_struct_u64->sign = false;
    ast_struct_append(&prog->structs, glob_struct_u64);
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

void resolve_symbols_num_lit(ast_prog *prog, ast_expr *expr) {
    char *data = expr->pointed.data;
    uint64_t val = 0;
    uint64_t base = 10;
    uint64_t digit;
    int i = 0;

    if(data[0] == '0') {
        switch(data[1]) {
        case 'x':
            base = 16;
            i = 2;
            break;
        case 'o':
            base = 8;
            i = 2;
            break;
        case 'b':
            base = 2;
            i = 2;
            break;
        default:
            break;
        }
    }

    for(; data[i]; i++) {
        if('0' <= data[i] && data[i] <= '9') {
            digit = data[i] - '0';
        } else if('a' <= data[i] && data[i] <= 'f') {
            digit = data[i] - 'a' + 10;
        } else if('A' <= data[i] && data[i] <= 'F') {
            digit = data[i] - 'A' + 10;
        } else if(data[i] == '_') {
            continue;
        } else if(data[i] == 'i' || data[i] == 'u') {
            // TODO dont search symbol
            expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, &data[i])->pointed.strct);
            break;
        } else {
            panic("invalid digit in number literal");
        }
        val *= base;
        val += digit;
    }
    if(!expr->type) {
        expr->type = ast_type_make_base(default_loc, glob_struct_i32);
    }
    free(data);
    if(expr->type->pointed.strct->sign) {
        expr->pointed.ival = val;
    } else {
        expr->pointed.uval = val;
    }
}

void resolve_symbols_expr(ast_prog *prog, ast_expr *expr) {
    ast_expr *subexpr;
    ast_symbol *symbol;
    bool is_const;
    switch(expr->vnt) {
    case AST_EXPR_IDENT:
        symbol = ast_symbol_find(prog, expr->pointed.data);
        if(symbol) {
            free(expr->pointed.data);
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
            case AST_SYMBOL_UNRESOLVED:
                panic("unresolved symbol in symbol stack");
                break;
            }
        } else {
            log_error("unknown symbol", expr->loc);
            exit(EXIT_FAILURE);
        }
        expr->is_const = false;
        break;
    case AST_EXPR_CHAR_LIT:
        // TODO calc value
        expr->type = ast_type_make_base(default_loc, glob_struct_char);
        expr->is_const = true;
        break;
    case AST_EXPR_STR_LIT:
        // TODO add to str list
        expr->type = ast_type_make_ref(default_loc, ast_type_make_slice(default_loc, ast_type_make_base(default_loc, glob_struct_char)));
        expr->is_const = true;
        break;
    case AST_EXPR_NUM_LIT:
        resolve_symbols_num_lit(prog, expr);
        expr->is_const = true;
        break;
    case AST_EXPR_TRUE:
        expr->pointed.uval = 1;
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        expr->is_const = true;
        break;
    case AST_EXPR_FALSE:
        expr->pointed.uval = 0;
        expr->type = ast_type_make_base(default_loc, glob_struct_bool);
        expr->is_const = true;
        break;
    case AST_EXPR_NULL:
        expr->pointed.uval = 0;
        expr->type = ast_type_make_ref(default_loc, ast_type_make_base(default_loc, glob_struct_void));
        expr->is_const = true;
        break;
    case AST_EXPR_TUPLE:
        subexpr = expr->rhs;
        is_const = true;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            if(subexpr->vnt == AST_EXPR_IDENT && subexpr->pointed_vnt != AST_SYMBOL_VAR) {
                log_error("invalid use of struct or function", subexpr->loc);
            }
            is_const = is_const && subexpr->is_const;
            subexpr = subexpr->next;
        }
        expr->is_const = is_const;
        break;
    case AST_EXPR_DOT:
        resolve_symbols_expr(prog, expr->lhs);
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt == AST_SYMBOL_FUNC) {
            log_error("functions don't have members", expr->lhs->loc);
        }
        expr->is_const = false;
        break;
    case AST_OP_CALL:
        resolve_symbols_expr(prog, expr->lhs);
        if(!((expr->lhs->vnt == AST_EXPR_IDENT && (expr->lhs->pointed_vnt == AST_SYMBOL_FUNC || expr->lhs->pointed_vnt == AST_SYMBOL_STRUCT)) || expr->lhs->vnt == AST_EXPR_DOT)) {
            log_error("not a function", expr->lhs->loc);
        }
        subexpr = expr->rhs;
        while(subexpr) {
            resolve_symbols_expr(prog, subexpr);
            if(subexpr->vnt == AST_EXPR_IDENT && subexpr->pointed_vnt != AST_SYMBOL_VAR) {
                log_error("invalid use of struct or function", subexpr->loc);
            }
            subexpr = subexpr->next;
        }
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt == AST_SYMBOL_STRUCT) {
            expr->is_const = true;
        } else {
            expr->is_const = false;
        }
        break;
    case AST_OP_INDEX:
        resolve_symbols_expr(prog, expr->lhs);
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->lhs->loc);
        }
        resolve_symbols_expr(prog, expr->rhs);
        if(expr->rhs->vnt == AST_EXPR_IDENT && expr->rhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->rhs->loc);
        }
        expr->is_const = false;
        break;
    case AST_OP_LOG_NOT:
    case AST_OP_BIT_NOT:
    case AST_OP_NEG:
        resolve_symbols_expr(prog, expr->rhs);
        if(expr->rhs->vnt == AST_EXPR_IDENT && expr->rhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->rhs->loc);
        }
        expr->is_const = expr->rhs->is_const;
        break;
    case AST_OP_REF:
    case AST_OP_DEREF:
    case AST_OP_ALLOC:
    case AST_OP_PUT:
    case AST_OP_TAKE:
        resolve_symbols_expr(prog, expr->rhs);
        if(expr->rhs->vnt == AST_EXPR_IDENT && expr->rhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->rhs->loc);
        }
        expr->is_const = false;
        break;
    case AST_OP_INC:
    case AST_OP_DEC:
        resolve_symbols_expr(prog, expr->lhs);
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->lhs->loc);
        }
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
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->lhs->loc);
        }
        resolve_symbols_expr(prog, expr->rhs);
        if(expr->rhs->vnt == AST_EXPR_IDENT && expr->rhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->rhs->loc);
        }
        expr->is_const = false;
        break;
    default:
        resolve_symbols_expr(prog, expr->lhs);
        if(expr->lhs->vnt == AST_EXPR_IDENT && expr->lhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->lhs->loc);
        }
        resolve_symbols_expr(prog, expr->rhs);
        if(expr->rhs->vnt == AST_EXPR_IDENT && expr->rhs->pointed_vnt != AST_SYMBOL_VAR) {
            log_error("invalid use of struct or function", expr->rhs->loc);
        }
        expr->is_const = (expr->lhs->is_const && expr->rhs->is_const);
        break;
    }
}

void resolve_symbols_type(ast_prog *prog, ast_type *type) {
    switch(type->vnt) {
    case AST_TYPE_BASE:
        break;
    case AST_TYPE_REF:
        resolve_symbols_type(prog, type->subtype);
        break;
    case AST_TYPE_ARR:
        resolve_symbols_expr(prog, type->arrlen);
        if(!type->arrlen->is_const) {
            log_error("value not known at compile time", type->arrlen->loc);
        }
        resolve_symbols_type(prog, type->subtype);
        break;
    case AST_TYPE_SLICE:
        resolve_symbols_type(prog, type->subtype);
        break;
    case AST_TYPE_TUPLE:
        type = type->subtype;
        while(type) {
            resolve_symbols_type(prog, type);
            type = type->next;
        }
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
            if(stmt->var->type) {
                resolve_symbols_type(prog, stmt->var->type);
            }
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
                exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
        resolve_symbols_type(prog, arg->type);
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
            if(!var->expr->is_const) {
                log_error("value not known at compile time", var->expr->loc);
            }
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
            type->pointed_vnt = AST_SYMBOL_STRUCT;
            type->pointed.strct = symbol->pointed.strct;
        } else {
            log_error("unknown type", type->loc);
            exit(EXIT_FAILURE);
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
    if(func->type) {
        resolve_types_type(prog, func->type);
    } else {
        func->type = ast_type_make_base(default_loc, glob_struct_void);
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
}
