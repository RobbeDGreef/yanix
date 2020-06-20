[global cpuid_cmd]
[global cpuid_available]

cpuid_available:
	push ebp
	mov ebp, esp

	pushfd
	pushfd

	xor dword [esp], 0x200000
	popfd
	pushfd
	pop eax
	xor eax, [esp]
	popfd
	and eax, 0x200000

	pop ebp
	ret


cpuid_cmd:
	push ebp
	mov ebp, esp
	
	mov eax, [ebp+12]
	cpuid

	push ebx
	mov ebx, [ebp+8]
	mov [ebx], eax
	mov eax, ebx

	pop ebx
	mov [eax+4], ebx
	mov [eax+8], ecx
	mov [eax+12], edx

	pop ebp
	ret