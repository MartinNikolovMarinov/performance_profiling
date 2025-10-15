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
;  LISTING 141
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global RATAdd
global RATMovAdd

section .text

RATAdd:
align 64
    mov     rax, 1000000000
.loop:
    ; Both ADDs write to the same register (rcx) consecutively.
    ; The second ADD depends on the result of the first.
    ; → This forms a true data dependency chain on rcx,
    ;   forcing each ADD to wait for the previous one to complete.
    add     rcx, 1
    add     rcx, 1

    ; Loop control
    dec     rax
    jnz     .loop
    ret


RATMovAdd:
align 64
    mov     rax, 1000000000
.loop:
    ; Each MOV breaks the dependency chain:
    ; rcx is overwritten with rax before every ADD,
    ; so the ADDs are independent and can issue in parallel.
    mov     rcx, rax
    add     rcx, 1
    mov     rcx, rax
    add     rcx, 1

    ; Loop control
    dec     rax
    jnz     .loop
    ret
