    .text
    .globl main
main:
    push %rbp
    mov %rsp, %rbp
    movslq %edi, %rdi
.Lmain0:
    cmp $0, %rdi
    jl .Lmain1
    pushq (%rsi,%rdi,8)
    dec %rdi
    jmp .Lmain0
.Lmain1:
    sub $8, %rsp
    call gnp_main
    pop %rax
    leave
    ret
