#ifndef ERROR_H
#define ERROR_H

#include "ast.h"

extern int errors;
extern int warnings;

void log_error(YYLTYPE, const char *);

#endif
