/* ts.s - startup, vectors, IRQ entry for Versatile PB */
.text
.code 32

.global reset
.global vectors_start, vectors_end
.global int_off, int_on
.global svc_stack, irq_stack

reset:
reset_handler:
    /* SVC stack */
    LDR sp, =svc_stack

    /* copy vector table to 0 */
    LDR r1, =vectors_start
    LDR r2, =vectors_end
    MOV r0, #0
1:
    LDR r3, [r1], #4
    STR r3, [r0], #4
    CMP r1, r2
    BNE 1b

    /* switch to IRQ mode, set IRQ stack */
    MRS r0, cpsr
    BIC r1, r0, #0x1F
    ORR r1, r1, #0x12
    MSR cpsr, r1
    LDR sp, =irq_stack

    /* back to SVC, enable IRQs */
    BIC r0, r0, #0x80
    MSR cpsr, r0

    BL main
    B .

/* IRQ entry */
.align 4
myhandler:
    SUB  lr, lr, #4
    STMFD sp!, {r0-r12, lr}
    BL   irq_handler
    LDMFD sp!, {r0-r12, pc}^

/* int_on / int_off (IRQ bit in CPSR) */
int_on:
    MRS r0, cpsr
    BIC r0, r0, #0x80
    MSR cpsr, r0
    MOV pc, lr

int_off:
    MRS r0, cpsr
    ORR r0, r0, #0x80
    MSR cpsr, r0
    MOV pc, lr

/* vector table */
vectors_start:
    LDR PC, reset_addr
    LDR PC, undef_addr
    LDR PC, swi_addr
    LDR PC, pabt_addr
    LDR PC, dabt_addr
    B   .
    LDR PC, irq_addr
    LDR PC, fiq_addr

reset_addr: .word reset_handler
undef_addr: .word hang
swi_addr:   .word hang
pabt_addr:  .word hang
dabt_addr:  .word hang
irq_addr:   .word myhandler
fiq_addr:   .word hang

vectors_end:

hang: B hang

.end
