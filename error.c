#include <stdio.h>
#include "lines.h"

int errors = 0;
int warnings = 0;

void log_error(int line, int start, int stop, const char *msg) {
    errors++;
    fprintf(stderr, "%s %d:%d-%d : %s\n", filename, line, start, stop - 1, msg);
    if(!ended_parsing) {
        fprintf(stderr, "%s\n", lines->str);
        int i;
        for(i = 1; i < start; i++) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "^");
        for(i = start + 1; i < stop; i++) {
            fprintf(stderr, "~");
        }
        fprintf(stderr, "\n");
    }
}
