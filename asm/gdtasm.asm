[GLOBAL gdt_flush]

gdt_flush:
	mov 	eax, [esp+4]
	lgdt 	[eax]

	mov 	ax, 0x10
	mov 	ds, ax
	mov 	es, ax
	mov 	fs, ax
	mov 	gs, ax
	mov 	ss, ax
	jmp 	0x08:.flush 	; 0x08 offset to code segment

.flush 
	ret

[GLOBAL tss_flush]
tss_flush:
	mov 	ax, 0x2B
	ltr 	ax
	ret