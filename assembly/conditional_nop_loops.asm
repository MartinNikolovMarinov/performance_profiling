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
;  LISTING 136
;  ========================================================================

;  ============================================================
;  Linux x86-64 System V ABI versions
;  ============================================================

global ConditionalNOP

section .text

; void ConditionalNOP(uint64_t count, uint8_t* data)
; System V ABI:
;   rdi = count (number of bytes)
;   rsi = pointer to data
ConditionalNOP:
    xor     rax, rax            ; i = 0
.loop:
    mov     r10, [rsi + rax]    ; v = data[i]
    inc     rax                 ; i++
    test    r10, 1              ; check if least significant bit of v is set
    jnz     .skip               ; if set (v odd), skip NOP
    nop                         ; execute NOP if even
.skip:
    cmp     rax, rdi            ; compare i < count
    jb      .loop               ; loop while i < count
    ret                         ; return
