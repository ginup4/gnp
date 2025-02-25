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
        printf("%%=");
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
        printf("%%");
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
    case AST_OP_ALLOC:
        printf("alloc");
        print_expr(expr->rhs);
        break;
    case AST_OP_REALLOC:
        print_expr(expr->lhs);
        printf("realloc");
        print_expr(expr->rhs);
        break;
    case AST_OP_PUT:
        printf("put");
        print_expr(expr->rhs);
        break;
    case AST_OP_TAKE:
        printf("take");
        print_expr(expr->rhs);
        break;
    case AST_OP_INC:
        print_expr(expr->lhs);
        printf("++");
        break;
    case AST_OP_DEC:
        print_expr(expr->lhs);
        printf("--");
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

void print_stmts(ast_stmt *stmt, int tab) {
    int i;
    ast_stmt *els;
    while(stmt) {
        for(i = 0; i < tab; i++) {
            printf("  ");
        }
        switch(stmt->vnt) {
        case AST_STMT_VAR:
            print_var(stmt->var);
            printf("\n");
            break;
        case AST_STMT_EXPR:
            print_expr(stmt->expr);
            printf("\n");
            break;
        case AST_STMT_RETURN:
            printf("return ");
            if(stmt->expr) {
                print_expr(stmt->expr);
            }
            printf("\n");
            break;
        case AST_STMT_BREAK:
            printf("break\n");
            break;
        case AST_STMT_CONTINUE:
            printf("continue\n");
            break;
        case AST_STMT_LOOP:
            printf("loop\n");
            print_stmts(stmt->body, tab + 1);
            break;
        case AST_STMT_WHILE:
            printf("while ");
            print_expr(stmt->expr);
            printf("\n");
            print_stmts(stmt->body, tab + 1);
            break;
        case AST_STMT_IF:
            printf("if ");
            print_expr(stmt->expr);
            printf("\n");
            print_stmts(stmt->body, tab + 1);
            els = stmt->els;
            while(els && els->vnt == AST_STMT_IF) {
                for(i = 0; i < tab; i++) {
                    printf("  ");
                }
                printf("elif ");
                print_expr(els->expr);
                printf("\n");
                print_stmts(els->body, tab + 1);
                els = els->els;
            }
            if(els) {
                for(i = 0; i < tab; i++) {
                    printf("  ");
                }
                printf("else\n");
                print_stmts(els->body, tab + 1);
            }
            break;
        case AST_STMT_FREE:
            printf("free ");
            print_expr(stmt->expr);
            printf("\n");
            break;
        }
        stmt = stmt->next;
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
    print_stmts(func->body, 1);
}

void print_struct(ast_struct *strct) {
    log_error(strct->loc, "ast error test: struct");
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
    log_error(impl->loc, "ast error test: impl");
    printf("impl '%s' {\n", impl->name);
    ast_func *func = impl->funcs;
    while(func) {
        print_func(func);
        func = func->next;
    }
    printf("}\n");
}
