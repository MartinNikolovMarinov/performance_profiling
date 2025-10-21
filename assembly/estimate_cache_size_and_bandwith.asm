global TestCacheSize

section .text

; void TestCacheSize(u8* data, u64 mask, u64 count);
; RDI, RSI, RDX

TestCacheSize:
    xor r9, r9

    align 64
.loop:
    add r9, 128
    and r9, rsi  ; & with the mask

    ; do the 128 byte read at rdi
    vmovdqu ymm0, [r9 + rdi]
    vmovdqu ymm1, [r9 + rdi + 32]
    vmovdqu ymm2, [r9 + rdi + 64]
    vmovdqu ymm3, [r9 + rdi + 96]

    dec rdx
    jg .loop

    ret
