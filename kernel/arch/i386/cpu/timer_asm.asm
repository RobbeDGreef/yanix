[global arch_timer_read_count]

arch_timer_read_count:
	push ebp
	mov ebp, esp

	xor eax, eax
	out 0x43, al

	in al, 0x40

	mov ah, al
	in al, 0x40
	rol ax, 8

	pop ebp
	ret