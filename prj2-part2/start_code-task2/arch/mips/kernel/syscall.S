#include "asm.h"

LEAF(invoke_syscall)
	syscall
	jr ra
    // TODO syscall entry
END(invoke_syscall)