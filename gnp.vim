if exists("b:current_syntax")
    finish
endif

" definitions
syn match gnpComment "#.*$"
syn match gnpInclude "^\$.*$"

syn keyword gnpKeyword fn let
syn keyword gnpKeyword struct impl
syn keyword gnpKeyword put take
syn keyword gnpKeyword alloc realloc free

" not used but might just as well be highlighted
syn keyword gnpKeyword as
syn keyword gnpStorage const
syn keyword gnpStorage pub

syn keyword gnpStatement if elif else
syn keyword gnpStatement loop while
syn keyword gnpStatement return break continue

syn keyword gnpType void char bool Self isize usize
syn keyword gnpType i8 u8 i16 u16 i32 u32 i64 u64 f32 f64

syn match gnpDecNumber display "\<[0-9][0-9_]*\%([iu]\%(size\|8\|16\|32\|64\)\)\="
syn match gnpHexNumber display "\<0x[a-fA-F0-9_]\+\%([iu]\%(size\|8\|16\|32\|64\)\)\="
syn match gnpOctNumber display "\<0o[0-7_]\+\%([iu]\%(size\|8\|16\|32\|64\)\)\="
syn match gnpBinNumber display "\<0b[01_]\+\%([iu]\%(size\|8\|16\|32\|64\)\)\="

syn keyword gnpSelf self
syn keyword gnpLiteral null true false

syn match gnpEscapeError display contained /\\./
syn match gnpStringEscape display contained /\\\([nrte0\\"]\)/
syn region gnpString start=+"+ end=+"+ contains=gnpStringEscape,gnpEscapeError
syn match gnpCharEscape display contained /\\\([nrt0\\']\)/
syn region gnpChar start=+'+ end=+'+ contains=gnpCharEscape,gnpEscapeError

syn match gnpOperator display "\%(+\|-\|/\|*\|=\|\^\|&\||\|!\|>\|<\|%\)=\?"
syn match gnpOperator display "&&\|||"

syn match gnpCall "\w\(\w\)*("he=e-1,me=e-1

" highlighting
hi def link gnpDecNumber gnpNumber
hi def link gnpHexNumber gnpNumber
hi def link gnpOctNumber gnpNumber
hi def link gnpBinNumber gnpNumber
hi def link gnpStorage storageClass
hi def link gnpNumber Number
hi def link gnpKeyword Keyword
hi def link gnpStatement Statement
hi def link gnpType Type
hi def link gnpSelf Constant
hi def link gnpLiteral Constant
hi def link gnpString Constant
hi def link gnpStringEscape Special
hi def link gnpChar Constant
hi def link gnpCharEscape Special
hi def link gnpEscapeError Error
hi def link gnpOperator Operator
hi def link gnpCall Function
hi def link gnpComment Comment
hi def link gnpInclude PreProc

let b:current_syntax = "gnp"
