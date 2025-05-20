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

struct location default_loc = {0};

void generate_base_types(ast_prog *prog) {
    ast_struct *strct;
    strct = ast_struct_create(default_loc, "void", NULL);
    strct->size = 0;
    strct->algn = 0;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "char", NULL);
    strct->size = 1;
    strct->algn = 1;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "bool", NULL);
    strct->size = 1;
    strct->algn = 1;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "isize", NULL);
    strct->size = 8; // architecture specific
    strct->algn = 8;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "usize", NULL);
    strct->size = 8; // architecture specific
    strct->algn = 8;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "i8", NULL);
    strct->size = 1;
    strct->algn = 1;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "u8", NULL);
    strct->size = 1;
    strct->algn = 1;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "i16", NULL);
    strct->size = 2;
    strct->algn = 2;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "u16", NULL);
    strct->size = 2;
    strct->algn = 2;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "i32", NULL);
    strct->size = 4;
    strct->algn = 4;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "u32", NULL);
    strct->size = 4;
    strct->algn = 4;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "i64", NULL);
    strct->size = 8;
    strct->algn = 8;
    ast_struct_append(&prog->structs, strct);
    strct = ast_struct_create(default_loc, "u64", NULL);
    strct->size = 8;
    strct->algn = 8;
    ast_struct_append(&prog->structs, strct);
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
        var->is_global = true;
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
            expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, &data[i])->pointed.strct);
            break;
        } else {
            panic("invalid digit in number literal");
        }
        val *= base;
        val += digit;
    }
    if(!expr->type) {
        expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, "i32")->pointed.strct);
    }
    free(data);
    expr->pointed.value = val;
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
        }
        expr->is_const = false;
        break;
    case AST_EXPR_CHAR_LIT:
        // TODO calc value
        expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, "char")->pointed.strct);
        expr->is_const = true;
        break;
    case AST_EXPR_STR_LIT:
        // TODO add to str list
        expr->type = ast_type_make_ref(default_loc, ast_type_make_slice(default_loc, ast_type_make_base(default_loc, ast_symbol_find(prog, "char")->pointed.strct)));
        expr->is_const = true;
        break;
    case AST_EXPR_NUM_LIT:
        resolve_symbols_num_lit(prog, expr);
        expr->is_const = true;
        break;
    case AST_EXPR_TRUE:
        expr->pointed.value = 1;
        expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, "bool")->pointed.strct);
        expr->is_const = true;
        break;
    case AST_EXPR_FALSE:
        expr->pointed.value = 0;
        expr->type = ast_type_make_base(default_loc, ast_symbol_find(prog, "bool")->pointed.strct);
        expr->is_const = true;
        break;
    case AST_EXPR_NULL:
        expr->pointed.value = 0;
        expr->type = ast_type_make_ref(default_loc, ast_type_make_base(default_loc, ast_symbol_find(prog, "void")->pointed.strct));
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
        expr->is_const = false;
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
}
