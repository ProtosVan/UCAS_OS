.text
.global main

main:
	la	$a0,	bootbegin
	lw	$t0,	printstr
	jal	$t0
	nop

	lw	$a0,	kernel
	li	$a1,	0x200
	lw	$t0,	kernelsize
	lw	$a2,	($t0)
	lw	$t0,	read_sd_card
	jal	$t0
	nop
	
	la	$a0,	bootdone
	lw	$t0,	printstr
	jal	$t0
	nop
	
	lw	$t0,	kernel
	jr	$t0
	nop
	
	la	$a0,	wrongmsg
	lw	$t0,	printstr
	jal	$t0
	nop

.data

bootbegin: .asciz "Boot by ProtosVan. All rights reserved.Version:0.1\n"

wrongmsg: .asciz "Something happended.\n"

bootdone: .asciz "Load kernel done. Begin kernel...\n"

# read_sd_card(address, offset, size);
read_sd_card: .word 0x80011000

# printstr(char *string)
printstr: .word 0x80011100

# printch(char ch)
printch: .word 0x80011140

kernel: .word 0xa0800200

kernelsize: .word 0xa08001f8
