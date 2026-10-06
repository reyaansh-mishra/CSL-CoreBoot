/* src/arch/arm64/vectors.s */

.section .text
.align 11   // Vector table alignment: 2KB (0x800)
.global vector_table
.global write_vbar_el2

.macro VENTRY label
    .align 7
    \label:
.endm

vector_table:
    /* --- Current EL, SP0 --- */
    VENTRY sync_sp0;     b vec_common
    VENTRY irq_sp0;      b vec_common
    VENTRY fiq_sp0;      b vec_common
    VENTRY serr_sp0;     b vec_common

    /* --- Current EL, SPx (EL2 -> EL2) --- */
    VENTRY sync_spx;     mov x5, #0; b vec_common
    VENTRY irq_spx;      mov x5, #1; b vec_common
    VENTRY fiq_spx;      mov x5, #2; b vec_common
    VENTRY serr_spx;     mov x5, #3; b vec_common

    /* --- Lower EL, AArch64 --- */
    VENTRY sync_lower64; b vec_common
    VENTRY irq_lower64;  b vec_common
    VENTRY fiq_lower64;  b vec_common
    VENTRY serr_lower64; b vec_common

    /* --- Lower EL, AArch32 --- */
    VENTRY sync_lower32; b vec_common
    VENTRY irq_lower32;  b vec_common
    VENTRY fiq_lower32;  b vec_common
    VENTRY serr_lower32; b vec_common

vec_common:
    // 1. Allocate stack frame (256 bytes)
    sub sp, sp, #256

    // 2. Save general-purpose registers x0 - x29, x30 (LR)
    stp x0,  x1,  [sp, #0]
    stp x2,  x3,  [sp, #16]
    stp x4,  x5,  [sp, #32]
    stp x6,  x7,  [sp, #48]
    stp x8,  x9,  [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30,      [sp, #240]

    // 3. Read System Registers into arguments x0-x3 for exception_dump
    mrs x0, esr_el2
    mrs x1, far_el2
    mrs x2, elr_el2
    mrs x3, spsr_el2
    mov x4, x5
    mov x5, sp          // Pass pointer to saved registers context as 5th arg

    // 4. Call C handler: void exception_dump(uint64_t esr, uint64_t far, uint64_t elr, uint64_t spsr, uint64_t *regs)
    bl exception_dump

halt:
    wfi
    b halt

write_vbar_el2:
    msr vbar_el2, x0
    isb
    ret