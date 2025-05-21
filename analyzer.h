#ifndef ANALYZER_H
#define ANALYZER_H

#include "ast.h"

extern ast_struct *glob_struct_void;
extern ast_struct *glob_struct_char;
extern ast_struct *glob_struct_bool;
extern ast_struct *glob_struct_isize;
extern ast_struct *glob_struct_usize;
extern ast_struct *glob_struct_i8;
extern ast_struct *glob_struct_u8;
extern ast_struct *glob_struct_i16;
extern ast_struct *glob_struct_u16;
extern ast_struct *glob_struct_i32;
extern ast_struct *glob_struct_u32;
extern ast_struct *glob_struct_i64;
extern ast_struct *glob_struct_u64;

void analyze_ast(ast_prog *);

#endif
