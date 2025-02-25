#ifndef LINES_H
#define LINES_H

#include <stdbool.h>

typedef struct line {
    char *str;
    struct line *next;
} line;

extern line *first_line;
extern line *last_line;
extern char *filename;

void append_line(char *);

#endif
