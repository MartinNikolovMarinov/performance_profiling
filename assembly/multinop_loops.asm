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
;  LISTING 135
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global NOP3x1AllBytes
global NOP1x3AllBytes
global NOP1x9AllBytes

section .text

NOP3x1AllBytes:
    xor     rax, rax              ; i = 0
.loop:
    db      0x0F, 0x1F, 0x00      ; 3-byte NOP
    inc     rax
    cmp     rax, rdi              ; i < Count
    jb      .loop
    ret

NOP1x3AllBytes:
    xor     rax, rax              ; i = 0
.loop:
    nop
    nop
    nop
    inc     rax                   ; i++
    cmp     rax, rdi              ; i < Count?
    jb      .loop
    ret

NOP1x9AllBytes:
    xor     rax, rax              ; i = 0
.loop:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    inc     rax                   ; i++
    cmp     rax, rdi              ; i < Count?
    jb .loop
    ret
