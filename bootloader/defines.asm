; DO NOT PUT CODE IN THIS FILE ONLY MACROS
; if you do the thing will blow up because the include for this file is at the top
; of the other files and the entry points won't be correct anymore

%define BLOCK_SIZE			512
%define MAX_SECTORS			0x80

%define SECOND_STAGE_ENTRY	0x9000
%define SECOND_STAGE_SIZE 	0x2000	; just a guess

%define KERNEL_ENTRY_POINT 	0x100000 	; 1 MB
%define KERNEL_SIZE 		0x10000 		; 60KB (just a guess)

%define ENABLE_VESA 		1
