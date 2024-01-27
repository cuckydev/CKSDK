# [ CKSDK ]
# Copyright 2024 Regan "CKDEV" Green
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

rom_start:

# OFFSET 0x0000
blob_addr:
.space 4
# OFFSET 0x0004
dest_addr:
.space 4
# OFFSET 0x0008
blob_end:
.space 4
# OFFSET 0x000C
blob_entry:
.space 4

# OFFSET 0x0010
entry:
	# This load gets patched by MkExe to point to the beginning of the
	# bootstrap blob. I've kept this split into lui and ori to keep
	# the patch unambiguous (li actually assemble to lui and oriu)
	lui   $t9, 0x0000
	ori   $t9, 0x0000

	# Ensure IRQs are disabled, as we're about to uninstall the stock BIOS
	# The BIOS should do this but some executable loaders may not
	mfc0  $t0, $12
	li    $t1, ~0x0401
	and   $t1, $t0
	mtc0  $t1, $12
	nop

	# Store the ROM address to FlushCache as we're going to be overriding that
	# We will use it, as well as CKSDK's OS
	la    $t8, 0x80000200 # BIOS A(nnh) Jump Table
	lw    $t7, (0x44 * 4)($t8)
	la    $t8, 0x80000000
	sw    $t7, 0($t8)

	# Decompress the blob
	lw    $a0, 0x0000($t9)
	lw    $a1, 0x0004($t9)
	lw    $a2, 0x0008($t9)
	# nop

# Implementation of the LZ4 decompressor
# a0 = in_p
# a1 = out_p
# a2 = in_e
lz4_decompress:
lz4_loop:
	# t0 = token = *in_p++
	lbu   $t0, 0($a0)
	addiu $a0, 1 # in_p++ for above

	# Perform literal copy
	# t1 = literal_length = (token >> 4) & 0xF
	srl   $t1, $t0, 4
	andi  $t1, 0xF

	# if literal_length == 0xF extend
	li    $at, 0xF
	bne   $t1, $at, literal_length_start
	li    $at, 0xFF

literal_length_addend_loop:
	# t2 = addend = in_p++
	lbu   $t2, 0($a0)
	addiu $a0, 1
	# if addend != 0xFF break
	beq   $t2, $at, literal_length_addend_loop
	# literal_length += addend
	addu  $t1, $t2

literal_length_start:
	# copy literal_length bytes from in_p to out_p
	beqz  $t1, literal_length_done
literal_length_loop:
	# literal_length--
	addiu $t1, -1
	# *a1++ = *a0++
	lbu   $t2, 0($a0)
	addiu $a0, 1
	sb    $t2, 0($a1)
	# if literal_length == 0 break
	bnez  $t1, literal_length_loop
	addiu $a1, 1

literal_length_done:
	# if in pointer matches the end, we're done here
	beq   $a0, $a2, lz4_done
	nop

	# Perform match copy
	# t1 = offset = in_p[0] | in_p[1] << 8
	lbu   $t1, 1($a0)
	addiu $a0, 2
	lbu   $t3, -2($a0)
	sll   $t1, 8
	or    $t1, $t3

	# t2 = copy_length = token & 0xF
	andi  $t2, $t0, 0xF

	# if copy_length == 0xF extend
	li    $at, 0xF
	bne   $t2, $at, copy_length_start
	li    $at, 0xFF

copy_length_addend_loop:
	# t3 = addend = in_p++
	lbu   $t3, 0($a0)
	addiu $a0, 1
	# if addend != 0xFF break
	beq   $t3, $at, copy_length_addend_loop
	# copy_length += addend
	addu  $t2, $t3

copy_length_start:
	# copy_length += 4
	addiu $t2, 4
	# t3 = copy_p = out_p - offset
	subu  $t3, $a1, $t1

	# copy copy_length bytes from copy_p to out_p
copy_length_loop:
	# copy_length--
	addiu $t2, -1
	# *a1++ = *t3++
	lbu   $t4, 0($t3)
	addiu $t3, 1
	sb    $t4, 0($a1)
	# if copy_length == 0 break
	bnez  $t2, copy_length_loop
	addiu $a1, 1

copy_length_done:
	b lz4_loop
	nop

lz4_done:
	# Move stack pointer to our reserved location
	li    $sp, (stack_top - rom_start)
	addu  $sp, $t9

	# Flush cache as we've modified executable contents of RAM
	jalr  $t7
	nop

	# Jump to loaded executable
	lw    $a0, 0x000C($t9)
	nop
	jr    $a0
	nop

stack:
# All we need is 4 bytes to store the return address when we call FlushCache
# FlushCache shouldn't require any stack space
.space 4
stack_top:
