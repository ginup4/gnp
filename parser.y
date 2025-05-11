%{
#include <stdlib.h>
#include <stdio.h>
#include "../ast.h"
#include "../error.h"
extern int yylex();
void yyerror(const char *);
%}

%locations

%define api.value.type union

%define parse.error detailed

// keywords
%token FN "'fn'"
%token LET "'let'"
%token STRUCT "'struct'"
%token IMPL "'impl'"

%token RETURN "'return'"
%token BREAK "'break'"
%token CONTINUE "'continue'"
%token IF "'if'"
%token ELIF "'elif'"
%token ELSE "'else'"
%token LOOP "'loop'"
%token WHILE "'while'"
%token FOR "'for'"

%token ALLOC "'alloc'"
%token REALLOC "'realloc'"
%token FREE "'free'"

%token PUT "'put'"
%token TAKE "'take'"

%token TRUE "'true'"
%token FALSE "'false'"
%token NUL "'null'"

// "value" tokens
%token <char*> IDENT "identifier"
%token <char*> NUM_LIT "number literal"
%token <char*> STR_LIT "string literal"
%token <char*> CHAR_LIT "char literal"

%destructor { free($$); } IDENT NUM_LIT STR_LIT CHAR_LIT

// multichar operator tokens
%token ADD_ASGN "'+='"
%token SUB_ASGN "'-='"
%token MUL_ASGN "'*='"
%token DIV_ASGN "'/='"
%token MOD_ASGN "'%='"
%token OR_ASGN "'|='"
%token AND_ASGN "'&='"
%token XOR_ASGN "'^='"

%token COMP_EQ "'=='"
%token COMP_NE "'!='"
%token COMP_LE "'<='"
%token COMP_GE "'>='"
%token COMP_LT "'<'"
%token COMP_GT "'>'"

%token LOG_OR "'||'"
%token LOG_AND "'&&'"

%token INC "'++'"
%token DEC "'--'"

// start symbol
%start program

// nonterminals
%nterm <ast_func*> func_decl
%nterm <ast_func*> func_decls
%nterm <ast_struct*> struct_decl
%nterm <ast_impl*> impl_decl
%nterm <ast_var*> arg_decls
%nterm <ast_var*> var_decl
%nterm <ast_type*> type
%nterm <ast_type*> typeseq
%nterm <ast_stmt*> stmts
%nterm <ast_stmt*> stmt
%nterm <ast_stmt*> loop_stmt
%nterm <ast_stmt*> while_stmt
%nterm <ast_stmt*> if_stmt
%nterm <ast_stmt*> elif_stmt
%nterm <ast_stmt*> else_stmt
%nterm <ast_expr*> callargs
%nterm <ast_expr*> exprseq
%nterm <ast_expr*> expr

%destructor { ast_func_free($$); } func_decl func_decls
%destructor { ast_struct_free($$); } struct_decl
%destructor { ast_impl_free($$); } impl_decl
%destructor { ast_var_free($$); } arg_decls var_decl
%destructor { ast_type_free($$); } type typeseq
%destructor { ast_stmt_free($$); } stmts stmt loop_stmt while_stmt if_stmt elif_stmt else_stmt
%destructor { ast_expr_free($$); } callargs exprseq expr

// operator precedence
%left REALLOC
%precedence ALLOC
%precedence PUT TAKE
%right '=' ADD_ASGN SUB_ASGN MUL_ASGN DIV_ASGN MOD_ASGN OR_ASGN AND_ASGN XOR_ASGN
%left LOG_OR
%left LOG_AND
%nonassoc COMP_EQ COMP_NE COMP_LE COMP_GE COMP_LT COMP_GT
%left '|'
%left '&' '^'
%left '+' '-'
%left '*' '/' '%'
%precedence P_NEG
%precedence INC DEC
%precedence P_REF
%left '(' '['
%left '.'

%%

program:
    %empty
|   program func_decl { ast_func_append(&glob_program.funcs, $2); }
|   program struct_decl { ast_struct_append(&glob_program.structs, $2); }
|   program impl_decl { ast_impl_append(&glob_program.impls, $2); }
|   program var_decl { ast_var_append(&glob_program.vars, $2); }
;

func_decl:
    FN IDENT '(' ')' '{' stmts '}' { $$ = ast_func_create(@2, $2, NULL, NULL, $6); }
|   FN IDENT '(' ')' ':' type '{' stmts '}' { $$ = ast_func_create(@2, $2, NULL, $6, $8); }
|   FN IDENT '(' arg_decls ')' '{' stmts '}' { $$ = ast_func_create(@2, $2, $4, NULL, $7); }
|   FN IDENT '(' arg_decls ')' ':' type '{' stmts '}' { $$ = ast_func_create(@2, $2, $4, $7, $9); }
;

func_decls:
    func_decl { $$ = $1; }
|   func_decls func_decl { ast_func_append(&$1->next, $2); $$ = $1; }
;

struct_decl:
    STRUCT IDENT '{' arg_decls '}' { $$ = ast_struct_create(@2, $2, $4); }
|   STRUCT IDENT '{' arg_decls ',' '}' { $$ = ast_struct_create(@2, $2, $4); }
;

impl_decl:
    IMPL IDENT '{' func_decls '}' { $$ = ast_impl_create(@2, $2, $4); }
;

arg_decls:
    IDENT ':' type { $$ = ast_var_create(@1, $1, $3, NULL); }
|   arg_decls ',' IDENT ':' type { ast_var_append(&$1->next, ast_var_create(@3, $3, $5, NULL)); $$ = $1; }
;

var_decl:
    LET IDENT ':' type ';' { $$ = ast_var_create(@2, $2, $4, NULL); }
|   LET IDENT ':' type '=' expr ';' { $$ = ast_var_create(@2, $2, $4, $6); }
|   LET IDENT '=' expr ';' { $$ = ast_var_create(@2, $2, NULL, $4); }
;

type:
    IDENT { $$ = ast_type_create(@$, $1); }
|   '(' typeseq ')' { $$ = ast_type_make_tuple(@$, $2); }
|   '&' type { $$ = ast_type_make_ref(@$, $2); }
|   '[' type ';' expr ']' { $$ = ast_type_make_arr(@$, $2, $4); }
|   '[' type ']' { $$ = ast_type_make_slice(@$, $2); }
;

typeseq:
    type ',' type { ast_type_append(&$1->next, $3); $$ = $1; }
|   typeseq ',' type { ast_type_append(&$1->next, $3); $$ = $1; }
;

stmts:
    stmt { $$ = $1; }
|   stmts stmt { ast_stmt_append(&$1->next, $2); $$ = $1; }
;

stmt:
    var_decl { $$ = ast_stmt_create(@$, AST_STMT_VAR, $1, NULL, NULL, NULL); }
|   expr ';' { $$ = ast_stmt_create(@$, AST_STMT_EXPR, NULL, $1, NULL, NULL); }
|   RETURN ';' { $$ = ast_stmt_create(@$, AST_STMT_RETURN, NULL, NULL, NULL, NULL); }
|   RETURN expr ';' { $$ = ast_stmt_create(@$, AST_STMT_RETURN, NULL, $2, NULL, NULL); }
|   BREAK ';' { $$ = ast_stmt_create(@$, AST_STMT_BREAK, NULL, NULL, NULL, NULL); }
|   CONTINUE ';' { $$ = ast_stmt_create(@$, AST_STMT_CONTINUE, NULL, NULL, NULL, NULL); }
|   FREE expr ';' { $$ = ast_stmt_create(@$, AST_STMT_FREE, NULL, $2, NULL, NULL); }
|   loop_stmt { $$ = $1; }
|   while_stmt { $$ = $1; }
|   if_stmt { $$ = $1; }
;

loop_stmt:
    LOOP '{' stmts '}' { $$ = ast_stmt_create(@$, AST_STMT_LOOP, NULL, NULL, $3, NULL); }
;

while_stmt:
    WHILE expr '{' stmts '}' { $$ = ast_stmt_create(@$, AST_STMT_WHILE, NULL, $2, $4, NULL); }
;

if_stmt:
    IF expr '{' stmts '}' { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, NULL); }
|   IF expr '{' stmts '}' elif_stmt { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, $6); }
|   IF expr '{' stmts '}' else_stmt { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, $6); }
;

elif_stmt:
    ELIF expr '{' stmts '}' { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, NULL); }
|   ELIF expr '{' stmts '}' elif_stmt { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, $6); }
|   ELIF expr '{' stmts '}' else_stmt { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, $2, $4, $6); }
;

else_stmt:
    ELSE '{' stmts '}' { $$ = ast_stmt_create(@$, AST_STMT_IF, NULL, NULL, $3, NULL); }
;

callargs:
    expr { $$ = $1; }
|   callargs ',' expr { ast_expr_append(&$1->next, $3); $$ = $1; }
;

exprseq:
    expr ',' expr { ast_expr_append(&$1->next, $3); $$ = $1; }
|   exprseq ',' expr { ast_expr_append(&$1->next, $3); $$ = $1; }
;

expr:
    IDENT { $$ = ast_expr_create(@$, AST_EXPR_IDENT, $1); }
|   NUM_LIT { $$ = ast_expr_create(@$, AST_EXPR_NUM_LIT, $1); }
|   STR_LIT { $$ = ast_expr_create(@$, AST_EXPR_STR_LIT, $1); }
|   CHAR_LIT { $$ = ast_expr_create(@$, AST_EXPR_CHAR_LIT, $1); }
|   TRUE { $$ = ast_expr_create(@$, AST_EXPR_TRUE, NULL); }
|   FALSE { $$ = ast_expr_create(@$, AST_EXPR_FALSE, NULL); }
|   NUL { $$ = ast_expr_create(@$, AST_EXPR_NULL, NULL); }
|   '(' expr ')' { $$ = $2; }
|   '(' exprseq ')' { $$ = ast_expr_make_tuple(@$, $2); }
|   expr '(' ')' { $$ = ast_expr_make_op(@$, AST_OP_CALL, $1, NULL); }
|   expr '(' callargs ')' { $$ = ast_expr_make_op(@$, AST_OP_CALL, $1, $3); }
|   expr '[' expr ']' { $$ = ast_expr_make_op(@$, AST_OP_INDEX, $1, $3); }
|   expr '.' IDENT { $$ = ast_expr_make_dot(@$, $1, $3); }
|   expr '.' NUM_LIT { $$ = ast_expr_make_dot(@$, $1, $3); }
|   expr '=' expr { $$ = ast_expr_make_op(@$, AST_OP_ASGN, $1, $3); }
|   expr ADD_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_ADD_ASGN, $1, $3); }
|   expr SUB_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_SUB_ASGN, $1, $3); }
|   expr MUL_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_MUL_ASGN, $1, $3); }
|   expr DIV_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_DIV_ASGN, $1, $3); }
|   expr MOD_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_MOD_ASGN, $1, $3); }
|   expr OR_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_OR_ASGN, $1, $3); }
|   expr AND_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_AND_ASGN, $1, $3); }
|   expr XOR_ASGN expr { $$ = ast_expr_make_op(@$, AST_OP_XOR_ASGN, $1, $3); }
|   expr COMP_EQ expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_EQ, $1, $3); }
|   expr COMP_NE expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_NE, $1, $3); }
|   expr COMP_LE expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_LE, $1, $3); }
|   expr COMP_GE expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_GE, $1, $3); }
|   expr COMP_LT expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_LT, $1, $3); }
|   expr COMP_GT expr { $$ = ast_expr_make_op(@$, AST_OP_COMP_GT, $1, $3); }
|   expr LOG_OR expr { $$ = ast_expr_make_op(@$, AST_OP_LOG_OR, $1, $3); }
|   expr LOG_AND expr { $$ = ast_expr_make_op(@$, AST_OP_LOG_AND, $1, $3); }
|   expr '+' expr { $$ = ast_expr_make_op(@$, AST_OP_ADD, $1, $3); }
|   expr '-' expr { $$ = ast_expr_make_op(@$, AST_OP_SUB, $1, $3); }
|   expr '*' expr { $$ = ast_expr_make_op(@$, AST_OP_MUL, $1, $3); }
|   expr '/' expr { $$ = ast_expr_make_op(@$, AST_OP_DIV, $1, $3); }
|   expr '%' expr { $$ = ast_expr_make_op(@$, AST_OP_MOD, $1, $3); }
|   expr '|' expr { $$ = ast_expr_make_op(@$, AST_OP_BIT_OR, $1, $3); }
|   expr '&' expr { $$ = ast_expr_make_op(@$, AST_OP_BIT_AND, $1, $3); }
|   expr '^' expr { $$ = ast_expr_make_op(@$, AST_OP_BIT_XOR, $1, $3); }
|   '-' expr %prec P_NEG { $$ = ast_expr_make_op(@$, AST_OP_NEG, NULL, $2); }
|   '!' expr %prec P_NEG { $$ = ast_expr_make_op(@$, AST_OP_LOG_NOT, NULL, $2); }
|   '~' expr %prec P_NEG { $$ = ast_expr_make_op(@$, AST_OP_BIT_NOT, NULL, $2); }
|   '&' expr %prec P_REF { $$ = ast_expr_make_op(@$, AST_OP_REF, NULL, $2); }
|   '*' expr %prec P_REF { $$ = ast_expr_make_op(@$, AST_OP_DEREF, NULL, $2); }
|   ALLOC expr { $$ = ast_expr_make_op(@$, AST_OP_ALLOC, NULL, $2); }
|   expr REALLOC expr { $$ = ast_expr_make_op(@$, AST_OP_REALLOC, $1, $3); }
|   PUT expr { $$ = ast_expr_make_op(@$, AST_OP_PUT, NULL, $2); }
|   TAKE expr { $$ = ast_expr_make_op(@$, AST_OP_TAKE, NULL, $2); }
|   expr INC { $$ = ast_expr_make_op(@$, AST_OP_INC, $1, NULL); }
|   expr DEC { $$ = ast_expr_make_op(@$, AST_OP_DEC, $1, NULL); }
;

%%

void yyerror(const char *msg) {
    log_error(msg, yylloc);
}
