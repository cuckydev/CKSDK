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

.extern main

.extern __bss_start
.extern _end

.extern __CTOR_LIST__

# void _start(void);
.section .text._start
.global _start
.type _start, @function
_start:
	# Initialize pointers
	li    $sp, 0x801FFFF0
	move  $fp, $sp

	# Clear BSS
	la    $t0, __bss_start
	la    $t1, (_end - 4)

.Lclear_bss_loop:
	sw    $zero, 0($t0)
	bne   $t0, $t1, .Lclear_bss_loop
	addiu $t0, 4

	# Run constructors
	# This is done in reverse order as the linker places them in reverse order
	la    $t0, __CTOR_LIST__
	lw    $t2, 0($t0)
	nop
	beqz  $t2, .Lno_ctor_loop
	sll   $t3, $t2, 2
	addu  $t0, $t0, $t3

.Lctor_loop:
	lw    $t1, 0($t0)
	addiu $t0, -4
	jalr  $t1
	addiu $t2, -1
	bnez  $t2, .Lctor_loop
	nop

.Lno_ctor_loop:
	# Call main
	j     main
	nop
