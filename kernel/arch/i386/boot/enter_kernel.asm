; this code will be placed at 0x10000 in memory and after jumping
; into 32 bit mode we will call 0x10000 (this code) to start executing kernel.c

global _start;
[bits 32]
_start:
	[extern _enter]
	;pop		eax		; data
	;pop		ebx		; code
	push 	esp
	call	_enter
	jmp		$