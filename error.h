#ifndef ERROR_H
#define ERROR_H

#include "ast.h"

extern int errors;
extern int warnings;

void log_error(const char *, YYLTYPE loc);
void log_warning(const char *, YYLTYPE loc);
void log_note(const char *, YYLTYPE loc);

#endif
