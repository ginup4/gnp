%{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "../error.h"
#include "../ast.h"
#include "../lines.h"
#include "parser.h"
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    yylloc.last_column += yyleng;
size_t line_input(char *, size_t);
#define YY_INPUT(buf, result, max_size) \
    result = line_input(buf, max_size);
%}

%%

#[^\n]*$                    /* comment */
^$[^\n]*$                   append_file(yytext + 1, yylloc); /* include */

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
\n                          yylloc.last_line++; yylloc.last_column = 1;

.                           return YYUNDEF;

%%

char *line_input_buf = NULL;
size_t line_input_cap = 0;
ssize_t line_input_len = 0;
size_t line_input_loc = 0;

size_t line_input(char *buf, size_t max_size) {
    if((ssize_t) line_input_loc >= line_input_len) {
        line_input_len = getline(&line_input_buf, &line_input_cap, yyin);
        if(line_input_len < 0) {
            return YY_NULL;
        }
        append_line(line_input_buf);
        line_input_loc = 0;
    }
    size_t bytes_avail = line_input_len - line_input_loc;
    if(bytes_avail > max_size) {
        memcpy(buf, line_input_buf + line_input_loc, max_size);
        line_input_loc += max_size;
        return max_size;
    } else {
        memcpy(buf, line_input_buf + line_input_loc, bytes_avail);
        line_input_loc += bytes_avail;
        return bytes_avail;
    }
}

int yywrap() {
    fclose(yyin);
    current_file = current_file->next;
    if(!current_file) {
        return 1;
    }
    yyin = fopen(current_file->name, "r");
    if(!yyin) {
        log_error(strerror(errno), current_file->loc);
        return 1;
    }
    current_file->line_offset = current_line_num;
    yylloc.first_line = 1;
    yylloc.first_column = 1;
    yylloc.last_line = 1;
    yylloc.last_column = 1;
    yylloc.file = current_file;
    return 0;
}
