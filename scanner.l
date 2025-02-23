%{
#include "../ast.h"
#include "parser.h"
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc.last_line++; \
            yylloc.last_column = 1; \
        } else { \
            yylloc.last_column++; \
        } \
    }
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

true                        return TRUE;
false                       return FALSE;
null                        return NUL;

[A-Za-z_]([A-Za-z_]|[0-9])* yylval.IDENT = strdup(yytext); return IDENT;
[0-9]([A-Za-z_]|[0-9])*     yylval.NUM_LIT = strdup(yytext); return NUM_LIT;
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

"("|")"|"{"|"}"|"["|"]"     return yytext[0];
"!"|"~"                     return yytext[0];
"%"|"*"|"-"|"+"|"/"         return yytext[0];
"|"|"^"|"&"                 return yytext[0];
"="                         return yytext[0];
"."                         return yytext[0];

[\n\t ]                     /* whitespace */

.                           return YYUNDEF;

%%

int yywrap() {}
