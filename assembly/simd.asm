;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 150
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global Read_4x2
global Read_8x2
global Read_16x2
global Read_32x2

section .text

Read_4x2:
    xor rax, rax
    align 64
.loop:
    mov r8d, [rsi ]
    mov r8d, [rsi + 4]
    add rax, 8
    cmp rax, rdi
    jb .loop
    ret

Read_8x2:
    xor rax, rax
    align 64
.loop:
    mov r8, [rsi ]
    mov r8, [rsi + 8]
    add rax, 16
    cmp rax, rdi
    jb .loop
    ret

Read_16x2:
    xor rax, rax
    align 64
.loop:
    vmovdqu xmm0, [rsi]
    vmovdqu xmm0, [rsi + 16]
    add rax, 32
    cmp rax, rdi
    jb .loop
    ret

Read_32x2:
    xor rax, rax
    align 64
.loop:
    vmovdqu ymm0, [rsi]
    vmovdqu ymm0, [rsi + 32]
    add rax, 64
    cmp rax, rdi
    jb .loop
    ret
