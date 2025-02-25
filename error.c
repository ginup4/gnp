#include <stdio.h>
#include "ast.h"
#include "lines.h"

int errors = 0;
int warnings = 0;

void log_error(YYLTYPE loc, const char *msg) {
    int fline = loc.first_line;
    int fcol = loc.first_column;
    int lline = loc.last_line;
    int lcol = loc.last_column;
    errors++;
    line *curr_line = first_line;
    int curr_line_ind = 1;
    if(fline == lline) {
        fprintf(stderr, "\e[31m%s %d:%d-%d : %s\e[m\n", filename, fline, fcol, lcol - 1, msg);
        while(curr_line_ind < fline && curr_line) {
            curr_line = curr_line->next;
            curr_line_ind++;
        }
        if(curr_line) {
            fprintf(stderr, "|   %s", curr_line->str);
            fprintf(stderr, "|   ");
            int i;
            for(i = 1; i < fcol; i++) {
                fprintf(stderr, " ");
            }
            fprintf(stderr, "\e[31m^");
            for(i = fcol + 1; i < lcol; i++) {
                fprintf(stderr, "~");
            }
            fprintf(stderr, "\e[m\n");
        }
    } else {
        fprintf(stderr, "\e[31m%s %d:%d-%d:%d : %s\e[m\n", filename, fline, fcol, lline, lcol - 1, msg);
    }
}
