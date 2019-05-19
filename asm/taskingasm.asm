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