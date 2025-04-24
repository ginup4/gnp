#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "lines.h"

int errors = 0;
int warnings = 0;

void _panic(const char *msg, const char *sourcefile, int linen) {
    fprintf(stderr, "\e[95mCompiler panic: %s\e[m\n", msg);
    fprintf(stderr, "%s: %d\n", sourcefile, linen);
    exit(EXIT_FAILURE);
}

void log_location(YYLTYPE loc) {
    int fline = loc.first_line;
    int fcol = loc.first_column;
    int lline = loc.last_line;
    int lcol = loc.last_column;
    line *curr_line = first_line;
    int curr_line_ind = 1;
    if(fline == lline) {
        fprintf(stderr, "%s: %d:%d-%d\n", filename, fline, fcol, lcol - 1);
        while(curr_line_ind < fline && curr_line) {
            curr_line = curr_line->next;
            curr_line_ind++;
        }
        if(curr_line && curr_line->str && curr_line->str[0] != '\0') {
            fprintf(stderr, "| %s", curr_line->str);
            if(curr_line->str[strlen(curr_line->str) - 1] != '\n') {
                fprintf(stderr, "\n");
            }
            fprintf(stderr, "| ");
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
        fprintf(stderr, "%s: %d:%d-%d:%d\n", filename, fline, fcol, lline, lcol - 1);
    }
}

void log_error(const char *msg, YYLTYPE loc) {
    fprintf(stderr, "\e[31mError: %s\e[m\n", msg);
    errors++;
    log_location(loc);
}

void log_warning(const char *msg, YYLTYPE loc) {
    fprintf(stderr, "\e[33mWarning: %s\e[m\n", msg);
    warnings++;
    log_location(loc);
}

void log_note(const char *msg, YYLTYPE loc) {
    fprintf(stderr, "\e[32mNote: %s\e[m\n", msg);
    log_location(loc);
}
