%include "../defines.asm"


[bits 16]
section 	.text

[global _start]
; Second stage entry
_start:
	; Save the boot drive
	pop 	dx
	and 	dx, 0xFF
	mov 	word [BOOT_DRIVE], dx

	; Save real mode ds
	push 	ds

	cli
 
	; Load gdt
	lgdt 	[gdt_descriptor]

	; switch to protected mode
	mov 	eax, cr0
	or 		eax, 1		; set protected mode bit (first bit)
	mov 	cr0, eax

	; Jump to the "protected mode code"
	jmp 	$ + 2

	; Select data descriptor
	mov 	bx, DATA_SEG
	mov 	ds, bx

	; Return to real mode by resetting that one bit
	and 	al, 0xFE
	mov 	cr0, eax

	jmp 	$+2

	pop 	ds

	; We now are in unreal mode
	
	mov 	bx, hello_test
	call 	print
	call 	print_newline

	call 	load_kernel


[global _jump_kernel]
[extern _findkernel]

load_kernel:


	; Check A20 and enable it if it isn't (qemu enables it by default i think but still)
	call 	check_a20

	mov 	bx, A20_enabled
	call 	print


	; Call c
 	push 	dword BLOCK_SIZE
 	push 	dword [BOOT_DRIVE]
 	
 	push 	word 0
 	call 	_findkernel
 	; Find kernel will jump to jump_kernel

_jump_kernel:
	mov 	bx, loadedKernel
	call 	print

	mov 	ax, 0x4f02
	mov 	bx, 0x0118
	int 	0x10
	
	call 	switch_to_pm

[bits 32]
pbegin:
	jmp 	KERNEL_ENTRY_POINT
	jmp 	$

[bits 16]
check_a20:
	; Since A20 is so poorly supported we need to 
	; check if it is enable in a few ways
	; First we'll try the bios method

	mov 	ax, 0x2403
	int 	0x15
	jb 		.bios_unsupported
	cmp 	ah, 0
	jnz 	.bios_unsupported

	; Check status using bios

	mov 	ax, 0x2402
	int 	0x15
	jb 		.bios_unsupported 	; Couldn't get status from bios
	cmp 	ah, 0
	jnz 	.bios_unsupported	; Couldn't get status from bios

	cmp 	al, 1
	jnz 	.bios_enable_a20	; The line is disabled

	; If we got here A20 is already enabled
	ret

.bios_enable_a20:
	mov 	ax, 0x2401
	int 	0x15
	jb 		.bios_unsupported 	; Couldn't activate a20
	cmp 	ah, 0
	jnz 	.bios_unsupported 	; Couldn't activate a20

	; Enabled just fine
	ret

.bios_unsupported:
	; @todo: More methods should be added

	ret

%include "gdt.asm"
%include "print.asm"
%include "load_block.asm"
%include "switch.asm"
; This is needed in the asm

section 	.rodata

hello_test: 	db "Unreal mode entered", 0x0
loadedKernel:	db "Kernel loaded, jumping...", 0x0
A20_enabled:	db "A20 line enabled", 0x0
testline: 		db "Test", 0x0
BOOT_DRIVE 		dd 0x0