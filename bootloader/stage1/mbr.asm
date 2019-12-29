%include "defines.asm"
%define SEG_INC_ON_READ ((MAX_SECTORS * BLOCK_SIZE) / 0x10)

[org 0x7c00]

; This code will be placed at 0x7c00 in memory and can only be 
; 0x1BB bytes long (443)

section .text

[bits 16]
_start:
	mov 	[BOOT_DRIVE], dl	; save the boot disk

	; Set up the stack
	mov 	bp, 0x9000	; stack size will be 4.5 KB
	mov 	sp, bp

	; kernel is supposed to reenable interrupts
	

	; Now we gotta copy the second stage into memory and jump to it 

	; Check whether bios has LBA extended mode
	mov 	ah, 0x41
	mov 	bx, 0x55aa
	mov 	dl, [BOOT_DRIVE] 
	int 	0x13
	jc 		.errorNoBios

	; Loading the second stage
	call 	load_second_stage

	mov 	bx, init_sec
	call 	print
	call 	print_newline

	; Now jumping to the second stage entry point
	
	push 	word [BOOT_DRIVE]
	jmp 	SECOND_STAGE_ENTRY
	;jmp 	(SECOND_STAGE_ENTRY / 0x10):(SECOND_STAGE_ENTRY % 0x10)

	.errorNoBios:
		mov 	bx, biosNoExtended
		call 	print
		jmp 	$

load_second_stage:
	
	mov 	word [high_tloc], (SECOND_STAGE_ENTRY / 0x10)
	mov 	dword [low_lba], 1

	; Set the loop counter
	mov  	cx, (SECOND_STAGE_SIZE / SEG_INC_ON_READ + 1)

	; A simple loop
	.reading_loop:

		; Check whether we should leave the loop
		cmp 	cx, 0
		jz 		.end

		; If not then decrease loop counter (cx)
		dec 	cx

		cmp 	cx, 5
		jnz 	.cont
		jmp 	$

	.cont:
		
		; Setting up some regissters for the read
		mov 	di, 0
		mov 	si, DISK_ADDRESS_PKT
		mov 	ah, 0x42
		mov 	dl, [BOOT_DRIVE]

		mov 	byte [DISK_ADDRESS_PKT], 	0x10
		mov 	word [blkcnt], 				MAX_SECTORS
		mov 	word [low_tloc], 			0
		mov 	word [high_lba], 			0

		; call bios function
		int 	0x13
		jc 		.readerror

		; Updating values after read
		mov 	bx, [low_lba]
		add 	bx, MAX_SECTORS
		mov 	[low_lba], bx

		mov 	bx, [high_tloc]
		add 	bx, SEG_INC_ON_READ
		mov 	[high_tloc], bx

		mov 	word dx, [high_tloc]
		call 	print_hex
		call 	print_newline

		; Loop again
		jmp 	.reading_loop

	.readerror:
		xor 	dx, dx
		mov 	dh, ah
		call 	print_hex
		mov 	bx, readerror
		call 	print
		jmp 	$

	.end:
		ret


; includes
%include "./stage1/print.asm"


DISK_ADDRESS_PKT:
			db	0x10	; size of packet (16 bytes)
			db 	0
blkcnt:		dw  MAX_SECTORS
low_tloc:	dw 	0
high_tloc: 	dw  (SECOND_STAGE_ENTRY / 0x10)
low_lba: 	dd 	1
high_lba: 	dd 	0

biosNoExtended: db "Bios does not support int13 extensions", 0x0
init_sec: 		db "Initializing second stage", 0x0
readerror: 		db " <- readerror", 0x0
ran: 			db " ran ", 0x0
BOOT_DRIVE: 	db 0x80
