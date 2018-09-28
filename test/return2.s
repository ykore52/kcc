.intel_syntax noprefix
.globl _main
_main:
    push rbp
    mov rbp,rsp
    mov rax,2
    mov rsp,rbp
    pop rbp
    ret
