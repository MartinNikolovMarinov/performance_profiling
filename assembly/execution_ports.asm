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
;  LISTING 144
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global Read_x1
global Read_x2
global Read_x3
global Read_x4
global Read_1x2
global Read_8x2

section .text

Read_x1:
    align 64
.loop:
    mov rax, [rsi]
    sub rdi, 1
    jnle .loop
    ret

Read_x2:
    align 64
.loop:
    mov rax, [rsi]
    mov rax, [rsi]
    sub rdi, 2
    jnle .loop
    ret

Read_x3:
    align 64
.loop:
    mov rax, [rsi]
    mov rax, [rsi]
    mov rax, [rsi]
    sub rdi, 3
    jnle .loop
    ret

Read_x4:
    align 64
.loop:
    mov rax, [rsi]
    mov rax, [rsi]
    mov rax, [rsi]
    mov rax, [rsi]
    sub rdi, 4
    jnle .loop
    ret

Read_1x2:
    align 64
.loop:
    mov al, [rsi]
    mov al, [rsi]
    sub rdi, 2
    jnle .loop
    ret
