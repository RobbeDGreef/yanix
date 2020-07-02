MBALIGN equ 1 << 0
MEMINFO equ 1 << 1
VIDEOSET equ 1 << 2
FLAGS  equ MBALIGN | MEMINFO | VIDEOSET
MAGIC equ 0x1badb002 			; Multiboot magic
CHECKSUM equ -(MAGIC + FLAGS)   ; The checksum of above

extern _enter

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

mb_mem_info:
	; Space for multiboot memory info
	dd 0 ; header address
	dd 0 ; load address
	dd 0 ; load end address
	dd 0 ; bss end address
	dd 0 ; entry address

mb_gfx_info:
	dd 0 	; mode_type
	dd 1024 ; width
	dd 768 	; height
	dd 32 	; depth

section .bss
align 16
	stack_bottom: resb 16*1024
	stack_top:

section .text
align 4
global _start
_start:
	; Setup our own stack
	mov esp, stack_top

	; EBX holds all the multiboot info
	push ebx
	push stack_top
	push esp

	; Call to C _enter(reg_t, reg_t, struct multiboot *) function (bootup.c)
	call _enter
	
	; never ever fall of into nowhere after kernel_main
	cli
	hlt
	jmp $
