# [ CKSDK ]
# Copyright 2023 Regan "CKDEV" Green
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

.set noreorder
.set noat

.extern CKSDK_OS_ISR_Callback
.type CKSDK_OS_ISR_Callback, @function

# IRQ globals
.extern CKSDK_OS_irq_recurse

# Thread global
.section .bss.CKSDK_OS_ISR_Thread
.align 4
CKSDK_OS_ISR_Thread:
	.space 34*4

# void CKSDK_OS_Syscall_DisableIRQ(void);
.section .text.CKSDK_OS_Syscall_DisableIRQ
.type CKSDK_OS_Syscall_DisableIRQ, @function
CKSDK_OS_Syscall_DisableIRQ:
	# Remove IRQ enable bit
	mfc0  $k0, $12
	li    $k1, ~0x0404
	and   $k0, $k1

	mtc0  $k0, $12
	nop

	# Get EPC and early exit from ISR
	mfc0  $k1, $14
	nop
	addiu $k1, 4 # Increment EPC so we don't run the syscall again

	jr    $k1
	rfe

# void CKSDK_OS_Syscall_EnableIRQ(void);
.section .text.CKSDK_OS_Syscall_EnableIRQ
.type CKSDK_OS_Syscall_EnableIRQ, @function
CKSDK_OS_Syscall_EnableIRQ:
	# Restore IRQ enable bit
	mfc0  $k0, $12
	nop
	ori   $k0, 0x0404
	mtc0  $k0, $12
	nop

	# Return to ISR
	b     .Lno_isr_syscall
	nop

# Syscall jumptable
.section .text.CKSDK_OS_ISR_syscall_jmptbl
CKSDK_OS_ISR_syscall_jmptbl:
	.word CKSDK_OS_Syscall_DisableIRQ
	.word CKSDK_OS_Syscall_EnableIRQ

# void CKSDK_OS_ISR(void);
.section .text.CKSDK_OS_ISR
.global CKSDK_OS_ISR
.type CKSDK_OS_ISR, @function
CKSDK_OS_ISR:
	# Check if this is a syscall
	mfc0  $k0, $13
	li    $k1, (8 << 2)
	andi  $k0, 0x7C
	bne   $k0, $k1, .Lno_isr_syscall
	
	# Jump to syscall
	la    $k0, CKSDK_OS_ISR_syscall_jmptbl
	sll   $k1, $a0, 2
	addu  $k0, $k1
	lw    $k1, 0($k0)
	nop
	jr    $k1
	nop

.Lno_isr_syscall:
	# Store thread state
	mfc0  $k1, $14  # Get exception program counter
	la    $k0, CKSDK_OS_ISR_Thread
	
	sw    $zero,  0*4($k0)
	sw    $at,    1*4($k0)
	sw    $v0,    2*4($k0)
	sw    $v1,    3*4($k0)
	sw    $a0,    4*4($k0)
	sw    $a1,    5*4($k0)
	sw    $a2,    6*4($k0)
	sw    $a3,    7*4($k0)
	sw    $t0,    8*4($k0)
	sw    $t1,    9*4($k0)
	sw    $t2,   10*4($k0)
	sw    $t3,   11*4($k0)
	sw    $t4,   12*4($k0)
	sw    $t5,   13*4($k0)
	sw    $t6,   14*4($k0)
	sw    $t7,   15*4($k0)
	sw    $s0,   16*4($k0)
	sw    $s1,   17*4($k0)
	sw    $s2,   18*4($k0)
	sw    $s3,   19*4($k0)
	sw    $s4,   20*4($k0)
	sw    $s5,   21*4($k0)
	sw    $s6,   22*4($k0)
	sw    $s7,   23*4($k0)
	sw    $t8,   24*4($k0)
	sw    $t9,   25*4($k0)
	sw    $k0,   26*4($k0)
	sw    $k1,   27*4($k0)
	sw    $gp,   28*4($k0)
	sw    $sp,   29*4($k0)
	sw    $fp,   30*4($k0)
	sw    $ra,   31*4($k0)

	# Load SR and cause
	mfc0  $t0, $12
	nop
	mfc0  $t1, $13
	
	sw    $t0, 32*4($k0)
	sw    $t1, 33*4($k0)

	# Increment IRQ recursion
	la    $t0, CKSDK_OS_irq_recurse
	lw    $t1, 0($t0)
	nop
	addiu $t1, 1
	sw    $t1, 0($t0)

	# Enter user ISR
	jal   CKSDK_OS_ISR_Callback
	move  $a0, $k0
	
	# Decrement IRQ recursion
	la    $t0, CKSDK_OS_irq_recurse
	lw    $t1, 0($t0)
	nop
	addiu $t1, -1
	sw    $t1, 0($t0)

	# Restore thread state
	# lw    $zero,  0*4($k0)
	lw    $at,    1*4($k0)
	lw    $v0,    2*4($k0)
	lw    $v1,    3*4($k0)
	lw    $a0,    4*4($k0)
	lw    $a1,    5*4($k0)
	lw    $a2,    6*4($k0)
	lw    $a3,    7*4($k0)
	lw    $t0,    8*4($k0)
	lw    $t1,    9*4($k0)
	lw    $t2,   10*4($k0)
	lw    $t3,   11*4($k0)
	lw    $t4,   12*4($k0)
	lw    $t5,   13*4($k0)
	lw    $t6,   14*4($k0)
	lw    $t7,   15*4($k0)
	lw    $s0,   16*4($k0)
	lw    $s1,   17*4($k0)
	lw    $s2,   18*4($k0)
	lw    $s3,   19*4($k0)
	lw    $s4,   20*4($k0)
	lw    $s5,   21*4($k0)
	lw    $s6,   22*4($k0)
	lw    $s7,   23*4($k0)
	lw    $t8,   24*4($k0)
	lw    $t9,   25*4($k0)
	# lw    $k0,   26*4($k0)
	lw    $k1,   27*4($k0)
	lw    $gp,   28*4($k0)
	lw    $sp,   29*4($k0)
	lw    $fp,   30*4($k0)
	lw    $ra,   31*4($k0)

	# Return from ISR
	jr    $k1
	rfe

# void CKSDK_OS_DisableIRQ_asm(void);
.section .text.CKSDK_OS_DisableIRQ_asm
.global CKSDK_OS_DisableIRQ_asm
.type CKSDK_OS_DisableIRQ_asm, @function
CKSDK_OS_DisableIRQ_asm:
	la    $t0, CKSDK_OS_irq_recurse
	lw    $t1, 0($t0)
	li    $a0, 0

	addiu $t2, $t1, 1
	
	bnez  $t1, .Lno_disable_syscall
	sw    $t2, 0($t0)

	syscall
	# nop

.Lno_disable_syscall:
	jr    $ra
	nop

# void CKSDK_OS_EnableIRQ_asm(void);
.section .text.CKSDK_OS_EnableIRQ_asm
.global CKSDK_OS_EnableIRQ_asm
.type CKSDK_OS_EnableIRQ_asm, @function
CKSDK_OS_EnableIRQ_asm:
	la    $t0, CKSDK_OS_irq_recurse
	lw    $t1, 0($t0)
	li    $a0, 1
	
	beqz  $t1, .Lbad_enable_irq

	addiu $t1, -1
	bnez  $t1, .Lno_enable_syscall
	sw    $t1, 0($t0)

	syscall
	# nop

.Lno_enable_syscall:
	jr    $ra
	nop

.Lbad_enable_irq:
	break
	jr    $ra
	nop
