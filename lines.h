#ifndef LINES_H
#define LINES_H

#include <stdbool.h>

typedef struct location {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    struct filename *file;
} location;

typedef struct line {
    char *str;
    struct line *next;
} line;

extern line *first_line;
extern line *last_line;
extern int current_line_num;

void append_line(char *);

typedef struct filename {
    char *name;
    struct filename *next;
    int line_offset;
    struct location loc;
} filename;

extern filename *first_file;
extern filename *current_file;
extern filename *last_file;

void append_file(char *, location);

#endif
