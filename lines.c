#include "lines.h"
#include <string.h>
#include <stdlib.h>

line *first_line = NULL;
line *last_line = NULL;
int current_line_num = 0;

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
    current_line_num++;
}

filename *first_file = NULL;
filename *current_file = NULL;
filename *last_file = NULL;

void append_file(char *name, location loc) {
    filename *new_file = malloc(sizeof(filename));
    new_file->name = strdup(name);
    new_file->next = NULL;
    new_file->loc = loc;
    last_file->next = new_file;
    last_file = new_file;
}
