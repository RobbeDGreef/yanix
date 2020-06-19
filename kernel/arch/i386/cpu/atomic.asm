[global arch_atomic_test_and_set]

arch_atomic_test_and_set:
	push ebp
	mov ebp, esp

	mov eax, 0
	mov ebx, [ebp+12]
	mov ecx, [ebp+8]

	lock cmpxchg [ecx], ebx
	; eax is either 0 -> succeeded or 1 -> was already locked
	pop ebp
	ret