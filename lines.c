#include "lines.h"
#include <string.h>
#include <stdlib.h>

line *first_line = NULL;
line *last_line = NULL;
char *filename;

void append_line(char *str) {
    line *new_line = malloc(sizeof(line));
    new_line->str = strdup(str);
    new_line->next = NULL;
    if(!first_line) {
        first_line = last_line = new_line;
    } else {
        last_line->next = new_line;
        last_line = new_line;
    }
}
