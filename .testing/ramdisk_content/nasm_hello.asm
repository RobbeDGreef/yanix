section .data
	MESSAGE: 	 db "Nasm program ran, and i know cuz im the program", 0x0
	MESSAGE_LEN: equ $ - MESSAGE

section .text
global 	_start

_start:
	mov 	eax, 4
	mov 	ebx, 1
	mov 	ecx, MESSAGE
	mov 	edx, MESSAGE_LEN
	int 	0x80

	mov 	eax, 1
	mov 	ebx, 2
	int 	0x80
