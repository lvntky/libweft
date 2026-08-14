    ; ==============================================================================
    ; WEFT v0.1.0
    ; coroutine tramboline base
    ; ===============================================================================


    ; ===============================================================================
    ; INITIZALIZED DATA
    ; ===============================================================================
section .data

section .bss
    ; ===============================================================================
    ; UNINITIALIZED DATA
    ; ===============================================================================

section .text
global coro

coro:

    ; --- System V AMD64 ABI Calling Convention (Linux/GCC x86_64) ---
    ; First argument (a) arrives automatically in:  RDI
    ; Second argument (b) arrives automatically in: RSI
    ; Return value must be placed back into:        RAX

    mov rax, rdi
    add rax, rsi

    ret 			;return control back to C
    
