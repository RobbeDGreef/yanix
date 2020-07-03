[global arch_atomic_test_and_set]
[global arch_atomic_compare_exchange]
[global arch_atomic_store]

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

arch_atomic_compare_exchange:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov ebx, [ebp+16]
	mov ecx, [ebp+8]

	lock cmpxchg [ecx], ebx

	pop ebp
	ret

arch_atomic_store:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov ebx, [ebp+8]

	lock mov [ebx], eax

	pop ebp
	ret