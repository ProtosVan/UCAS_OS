#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
.endm

.macro CLI
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
END(asm_start)



LEAF(OPEN_INTER)
    li      k0, 0x01000000
    mtc0    zero, CP0_COUNT
    mtc0    k0, CP0_COMPARE
    nop
    mfc0    k0, CP0_STATUS
    nop
    li      k1, 0x8001
    or      k0, k0, k1
    mtc0    k0, CP0_STATUS
    nop
    mfc0    k1, CP0_STATUS
    nop
    jr      ra
    nop
END(OPEN_INTER)

LEAF(CLOSE_INTER)
    mfc0    k0, CP0_STATUS
	nop
	li      k1, 0xfffffffe
	and     k0, k0, k1
	mtc0    k0, CP0_STATUS
	nop

    jr      ra
    nop
END(CLOSE_INTER)

LEAF(RESET_COUNPARE)
    li      k0, 0x01000000
    mtc0    zero, CP0_COUNT
    mtc0    k0, CP0_COMPARE
    jr      ra
    nop
END(RESET_COUNPARE)

.macro SAVE_CONTEXT offset
    #   $k0(26)     current_running
    lw		k0, current_running    # Current running is a global pointer.
    sw		$1, OFFSET_REG1(k0)
    sw		$2, OFFSET_REG2(k0)
    sw		$3, OFFSET_REG3(k0)
    sw		$4, OFFSET_REG4(k0)
    sw		$5, OFFSET_REG5(k0)
    sw		$6, OFFSET_REG6(k0)
    sw		$7, OFFSET_REG7(k0)
    sw		$8, OFFSET_REG8(k0)
    sw		$9, OFFSET_REG9(k0)
    sw		$10, OFFSET_REG10(k0)
    sw		$11, OFFSET_REG11(k0)
    sw		$12, OFFSET_REG12(k0)
    sw		$13, OFFSET_REG13(k0)
    sw		$14, OFFSET_REG14(k0)
    sw		$15, OFFSET_REG15(k0)
    sw		$16, OFFSET_REG16(k0)
    sw		$17, OFFSET_REG17(k0)
    sw		$18, OFFSET_REG18(k0)
    sw		$19, OFFSET_REG19(k0)
    sw		$20, OFFSET_REG20(k0)
    sw		$21, OFFSET_REG21(k0)
    sw		$22, OFFSET_REG22(k0)
    sw		$23, OFFSET_REG23(k0)
    sw		$24, OFFSET_REG24(k0)
    sw		$25, OFFSET_REG25(k0)
    # no    26
    # no    27
    sw		$28, OFFSET_REG28(k0)
    sw		$29, OFFSET_REG29(k0)
    sw		$30, OFFSET_REG30(k0)
    sw		$31, OFFSET_REG31(k0)

    mfc0    k1, CP0_STATUS
    nop
    sw      k1, OFFSET_STATUS(k0)

    mfhi    k1
    nop
    sw      k1, OFFSET_HI(k0)

    mflo    k1
    nop
    sw      k1, OFFSET_LO(k0)

    mfc0    k1, CP0_BADVADDR
    nop
    sw      k1, OFFSET_BADVADDR(k0)

    mfc0    k1, CP0_CAUSE
    nop
    sw      k1, OFFSET_CAUSE(k0)

    mfc0    k1, CP0_EPC
    nop
    sw      k1, OFFSET_EPC(k0)

.endm

.macro RESTORE_CONTEXT offset
        #   $k0(26)     current_running
    lw		k0, current_running    # Current running is a global pointer.

    lw		$1, OFFSET_REG1(k0)
    lw		$2, OFFSET_REG2(k0)
    lw		$3, OFFSET_REG3(k0)
    lw		$4, OFFSET_REG4(k0)
    lw		$5, OFFSET_REG5(k0)
    lw		$6, OFFSET_REG6(k0)
    lw		$7, OFFSET_REG7(k0)
    lw		$8, OFFSET_REG8(k0)
    lw		$9, OFFSET_REG9(k0)
    lw		$10, OFFSET_REG10(k0)
    lw		$11, OFFSET_REG11(k0)
    lw		$12, OFFSET_REG12(k0)
    lw		$13, OFFSET_REG13(k0)
    lw		$14, OFFSET_REG14(k0)
    lw		$15, OFFSET_REG15(k0)
    lw		$16, OFFSET_REG16(k0)
    lw		$17, OFFSET_REG17(k0)
    lw		$18, OFFSET_REG18(k0)
    lw		$19, OFFSET_REG19(k0)
    lw		$20, OFFSET_REG20(k0)
    lw		$21, OFFSET_REG21(k0)
    lw		$22, OFFSET_REG22(k0)
    lw		$23, OFFSET_REG23(k0)
    lw		$24, OFFSET_REG24(k0)
    lw		$25, OFFSET_REG25(k0)
    # no    26
    # no    27
    lw		$28, OFFSET_REG28(k0)
    lw		$29, OFFSET_REG29(k0)
    lw		$30, OFFSET_REG30(k0)
    lw		$31, OFFSET_REG31(k0)

    lw      k1, OFFSET_STATUS(k0)
    mtc0    k1, CP0_STATUS
    nop

    lw      k1, OFFSET_HI(k0)
    mthi    k1
    nop

    lw      k1, OFFSET_LO(k0)
    mtlo    k1
    nop

    lw      k1, OFFSET_BADVADDR(k0)
    mtc0    k1, CP0_BADVADDR
    nop

    lw      k1, OFFSET_CAUSE(k0)
    mtc0    k1, CP0_CAUSE
    nop

    lw      k1, OFFSET_EPC(k0)
    mtc0    k1, CP0_EPC
    nop

.endm

.set    noat
.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    mfc0    k0, CP0_EPC
    nop
    mfc0    k0, CP0_STATUS
    nop
    li      k1, 0xfffffffe
    and     k0, k0, k1
    mtc0    k0, CP0_STATUS
    nop

testsave:

    SAVE_CONTEXT(USER)
    li      sp, 0xa0f05000

    mfc0    k0, CP0_CAUSE
    nop
    andi    k0, 0x7c
    la      k1, exception_handler
    add     k0, k0, k1
    lw      k0, 0(k0)
    jr      k0
    nop
    nop
    //TODO close interrupt
    // jmp exception_handler[i] which decided by CP0_CAUSE
    // Leve2 exception Handler.
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    mfc0    a0, CP0_STATUS
    mfc0    a1, CP0_CAUSE
    nop
    addi	sp, sp, -8			# sp = sp + -8
    
    jal		interrupt_helper				# jump to interrupt_helper
    addi    sp, sp, 8
    nop
    RESTORE_CONTEXT(USER)
    li      k0, 0x01000000
    mtc0    zero, CP0_COUNT
    nop
    mtc0    k0, CP0_COMPARE
    nop
    mfc0    k1, CP0_STATUS
    nop
    li      k0, 0x1
    or      k1, k1, k0
    mtc0    k1, CP0_STATUS
    nop
    eret
    nop

    // interrupt handler
    // Leve3 exception Handler.
END(handle_int)

NESTED(handle_syscall, 0, sp)
    addi    sp, sp, -16
    jal		system_call_helper
    addi    sp, sp, 16

    RESTORE_CONTEXT(USER)
    li      k0, 0x01000000
    mtc0    k0, CP0_COMPARE
    mtc0    zero, CP0_COUNT
    nop
    mfc0    k0, CP0_STATUS
    li      k1, 0x00000001
	or      k0, k0, k1
	mtc0    k0, CP0_STATUS
	eret
    // system call handler
END(handle_syscall)

NESTED(handle_other, 0, sp)
    j		0x0				# jump to 0x0
    
    // other exception handler
END(handle_other)
