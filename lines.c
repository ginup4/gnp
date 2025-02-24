#include "lines.h"
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdbool.h>

line *lines = NULL;
char *filename;
bool ended_parsing = false;

void extend_line(char *str) {
    lines->str = realloc(lines->str, strlen(lines->str) + strlen(str) + 1);
    strcat(lines->str, str);
}

void next_line() {
    line *new_line = malloc(sizeof(line));
    new_line->str = malloc(1);
    new_line->str[0] = '\0';
    new_line->next = lines;
    lines = new_line;
}

void reverse_lines() {
    ended_parsing = true;
    line *new_lines = NULL;
    line *temp;
    while(lines) {
        temp = lines;
        lines = lines->next;
        temp->next = new_lines;
        new_lines = temp;
    }
    lines = new_lines;
}
