#ifndef ERROR_H
#define ERROR_H

#include "ast.h"

extern int errors;
extern int warnings;

#define panic(msg) _panic((msg), __FILE__, __LINE__)

void _panic(const char *, const char *, int);
void log_error(const char *, YYLTYPE loc);
void log_warning(const char *, YYLTYPE loc);
void log_note(const char *, YYLTYPE loc);

#endif
