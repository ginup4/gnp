#ifndef ERROR_H
#define ERROR_H

extern int errors;
extern int warnings;

void log_error(int, int, int, const char *);

#endif
