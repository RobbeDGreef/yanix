[GLOBAL get_eip]
get_eip:
	pop eax
	jmp	eax

[GLOBAL task_switch]
task_switch: ;task_switch_new(eip, esp, ebp, cr3)
	cli
	mov ecx, [esp+4]  ; eip
	mov eax, [esp+16] ; cr3
	mov ebp, [esp+12] ; ebp
	mov esp, [esp+8]  ; esp
	mov cr3, eax      ; load esp
	mov eax, 0
	sti
	jmp ecx 		  ; jump to eip

[GLOBAL jmp_userspace]
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
