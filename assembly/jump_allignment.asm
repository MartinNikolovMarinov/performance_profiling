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
;  LISTING 139
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global NOPAligned64
global NOPAligned1
global NOPAligned15
global NOPAligned31
global NOPAligned63

section .text

NOPAligned64:
    xor     rax, rax    ; i = 0
align 64
.loop:
    inc     rax         ; i++
    cmp     rax, rdi    ; compare i < count
    jb      .loop       ; loop while i < count
    ret

NOPAligned1:
    xor rax, rax
align 64
nop
.loop:
    inc     rax         ; i++
    cmp     rax, rdi    ; compare i < count
    jb      .loop       ; loop while i < count
    ret

NOPAligned15:
    xor rax, rax
align 64
%rep 15
nop
%endrep
.loop:
    inc     rax         ; i++
    cmp     rax, rdi    ; compare i < count
    jb      .loop       ; loop while i < count
    ret

NOPAligned31:
    xor rax, rax
align 64
%rep 31
nop
%endrep
.loop:
    inc     rax         ; i++
    cmp     rax, rdi    ; compare i < count
    jb      .loop       ; loop while i < count
    ret

NOPAligned63:
    xor rax, rax
align 64
%rep 63
nop
%endrep
.loop:
    inc     rax         ; i++
    cmp     rax, rdi    ; compare i < count
    jb      .loop       ; loop while i < count
    ret
