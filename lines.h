#ifndef LINES_H
#define LINES_H

#include <stdbool.h>

typedef struct line {
    char *str;
    struct line *next;
} line;

extern line *lines;
extern char *filename;
extern bool ended_parsing;

void extend_line(char *);
void next_line();
void reverse_lines();

#endif
