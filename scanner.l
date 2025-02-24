%{
#include "../ast.h"
#include "parser.h"
#include "../lines.h"
#include <string.h>
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    yylloc.last_column += strlen(yytext); \
    extend_line(yytext);
%}

%%

#[^\n]*$                 /* comment */

fn                          return FN;
let                         return LET;
struct                      return STRUCT;
impl                        return IMPL;

return                      return RETURN;
break                       return BREAK;
continue                    return CONTINUE;
if                          return IF;
elif                        return ELIF;
else                        return ELSE;
loop                        return LOOP;
while                       return WHILE;
for                         return FOR;

alloc                       return ALLOC;
realloc                     return REALLOC;
free                        return FREE;

put                         return PUT;
take                        return TAKE;

true                        return TRUE;
false                       return FALSE;
null                        return NUL;

 /* reserved keywords */
pub                         return YYUNDEF;
const                       return YYUNDEF;
as                          return YYUNDEF;

[A-Za-z_]([A-Za-z_]|[0-9])* yylval.IDENT = strdup(yytext); return IDENT;

[0-9][0-9_]*([iu](size|8|16|32|64))? yylval.NUM_LIT = strdup(yytext); return NUM_LIT;
0x[a-fA-F0-9_]*([iu](size|8|16|32|64))? yylval.NUM_LIT = strdup(yytext); return NUM_LIT;
0o[0-7_]*([iu](size|8|16|32|64))? yylval.NUM_LIT = strdup(yytext); return NUM_LIT;
0b[01_]*([iu](size|8|16|32|64))? yylval.NUM_LIT = strdup(yytext); return NUM_LIT;
[0-9]([A-Za-z_]|[0-9])*     return YYUNDEF; /* invalid number literal */

\"([^\n\"\\]|\\[^\n])*\"    yylval.STR_LIT = strdup(yytext); return STR_LIT;
'([^\n'\\]|\\[^\n])+\'      yylval.CHAR_LIT = strdup(yytext); return CHAR_LIT;

","                         return yytext[0];
";"                         return yytext[0];
":"                         return yytext[0];

"+="                        return ADD_ASGN;
"-="                        return SUB_ASGN;
"*="                        return MUL_ASGN;
"/="                        return DIV_ASGN;
"%="                        return MOD_ASGN;
"|="                        return OR_ASGN;
"&="                        return AND_ASGN;
"^="                        return XOR_ASGN;

"=="                        return COMP_EQ;
"!="                        return COMP_NE;
"<="                        return COMP_LE;
">="                        return COMP_GE;
"<"                         return COMP_LT;
">"                         return COMP_GT;

"||"                        return LOG_OR;
"&&"                        return LOG_AND;

"++"                        return INC;
"--"                        return DEC;

"("|")"|"{"|"}"|"["|"]"     return yytext[0];
"!"|"~"                     return yytext[0];
"%"|"*"|"-"|"+"|"/"         return yytext[0];
"|"|"^"|"&"                 return yytext[0];
"="                         return yytext[0];
"."                         return yytext[0];

[\t ]                       /* whitespace */
\n                          yylloc.last_line++; yylloc.last_column = 1; next_line();

.                           return YYUNDEF;

%%

int yywrap() {}
