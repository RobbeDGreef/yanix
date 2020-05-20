[GLOBAL get_eip]
get_eip:
	pop eax
	jmp	eax

[global arch_spawn_task]
[extern duplicate_current_page_directory]
; void arch_spawn_task(reg_t *esp, page_directory_t **dir)

arch_spawn_task:
	cli
	pushad

	; Save esp
	mov 	eax, [esp + (8 + 1) * 4]
	mov 	[eax], esp

	; Now create a new address space and thus copy the stack
	call 	duplicate_current_page_directory
	mov 	ebx, [esp + (8 + 2) * 4]
	mov 	[ebx], eax

	; Now clean the stack up
	popad
	ret


; so push all registers and put the return address to the given eip


[global do_task_switch]
do_task_switch: ; void do_task_switch(reg_t *previous_esp, reg_t next_esp, reg_t cr3);
	cli
	pushad
	
	; pusha pushed 8 resigesters  
	mov 	eax, [esp + (8 + 1) * 4]
	mov 	[eax], esp

	mov 	eax, [esp + (8 + 2) * 4]
	mov 	ebx, [esp + (8 + 3) * 4]
	mov 	ecx, [esp + (8 + 4) * 4]
	mov 	esp, eax
	mov 	cr3, ebx
	;cmp 	ecx, 0xc002a000
	;je 		$

	popad
	
	ret

[global jmp_userspace]
jmp_userspace: ;jmp_userspace(uint32_t eip, uint32_t argc, uint32_t argv)
	cli
	mov 	ebx, [esp+4]	; holds eip
	mov 	ecx, [esp+8]	; holds argument count
	mov 	edx, [esp+12] 	; holds argument variables

	mov 	ax, 0x23
	mov 	ds, ax
	mov 	es, ax
	mov 	fs, ax
	mov 	gs, ax

	push 	edx
	push 	ecx

	mov 	eax, esp
	push 	dword 0x23
	push 	dword eax
	sti
	pushfd
	cli
	pop 	eax
	or 		eax, 0x200  ; reenable IF in EFLAGS so that interrupts will work
	push 	eax
	push 	dword 0x1B
	push 	ebx
	iret 						; start at our given eip

[global arch_flush_tlb]
arch_flush_tlb:
	mov 	eax, cr3
	mov 	cr3, eax
	ret