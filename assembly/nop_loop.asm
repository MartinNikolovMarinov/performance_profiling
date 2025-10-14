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
;  LISTING 132
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global MOVAllBytesASM
global NOPAllBytesASM
global CMPAllBytesASM
global DECAllBytesASM

section .text

MOVAllBytesASM:
    xor     rax, rax                ; i = 0
.loop:
    mov     byte [rdi + rax], al    ; Data[i] = (u8)i
    inc     rax
    cmp     rdi, rax                ; i >= Count?
    jne      .loop
    ret

NOPAllBytesASM:
    mov     rcx, rdi              ; loop counter = Count
.loop:
    db      0x0F, 0x1F, 0x00      ; 3-byte NOP
    dec     rcx                   ; i--
    jnz     .loop
    ret

CMPAllBytesASM:
    mov     rcx, rdi              ; loop counter = Count
.loop:
    cmp     rcx, rcx              ; dummy compare (no memory access)
    dec     rcx
    jnz     .loop
    ret

DECAllBytesASM:
.loop:
    dec     rdi                   ; Count--
    jnz     .loop
    ret
