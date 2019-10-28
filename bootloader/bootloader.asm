; Tell the linker that this code is at 0x7c00 in memory
[org 0x7c00]

; define some constants
%define RAMDISK_ENTRY  	0x10000
%define RAMDISK_SIZE  	0x60000
%define KERNEL_ENTRY	RAMDISK_ENTRY + RAMDISK_SIZE
%define KERNEL_SIZE		0x20000
%define TOTAL_SIZE 		KERNEL_SIZE + RAMDISK_SIZE

%define MAX_SECTORS		0x80
%define SECTOR_SIZE 	0x200	; NOTE: we calculate this on runtime this value is just as a backup
%define BYTES_ON_W		MAX_SECTORS * SECTOR_SIZE
%define SEG_ON_WRITE	BYTES_ON_W / 0x10
%define WRITE_CYCLES	TOTAL_SIZE / BYTES_ON_W

section 	.text

	mov 	[BOOT_DRIVE], dl
	mov 	bp, 0x8000
	mov 	sp, bp


	call 	load_kernel

	mov 	bx, loadedSuccessfull
	call 	print

	mov     ax, 0x4f02
    mov     bx, 0x4118      ; in qemu 24 bit and LFB is 0xfd000000 (yes i know i should call 0xf01 but this works and it just is a hobby project)
    int     0x10
    
    call 	switch_to_pm
	jmp 	$


%include "./print.asm"
%include "./gdt/gdt.asm"
%include "./gdt/switch.asm"


[bits 16]
load_kernel:

	; check if bios has LBA extended mode
	mov 	ah, 0x41
	mov 	bx, 0x55aa
	mov 	dl, 0x80
	int 	0x13
	jc 		.error

	mov 	word [high_tloc], (RAMDISK_ENTRY/0x10) ; this is the segment pointer so this will be multiplied with 0x10
	mov 	dword [low_lba],  1
	mov 	cx, 9	; amount of loops 
	

	.writing_loop:

		cmp 	cx, 0
		jz 		.end

		; looping again so decline cx
		dec 	cx

		; resetting some registers just to because we are not sure if the bios edits them
		mov 	di, 0
		mov 	si, DA_PACKET
		mov 	ah, 0x42
		mov 	dl, [BOOT_DRIVE]

		; same here
		mov 	byte [DA_PACKET], 	0x10
		mov 	word [blkcnt], 		MAX_SECTORS
		mov 	word [low_tloc], 	0
		mov 	word [high_lba], 	0

		int 	0x13
		;jc 		.error

		; Call print to tell us we ran a successfull read
		mov 	bx, loadedSuccessfull
		call 	print

		; now let's update our values for our next write
		mov 	bx, [low_lba]
		add 	bx, MAX_SECTORS
		mov 	[low_lba], bx

		mov 	bx, [high_tloc]
		add 	bx, SEG_ON_WRITE
		mov 	[high_tloc], bx

		; now jump to start of loop
		jmp .writing_loop 


	.end:
		ret


	.error:
		mov 	bx, errorCode
		call 	print

		xor 	bx, bx
		mov 	bl, ah
		call 	print_hex
		
		jmp $


[bits 32]
pm_begin:
    push    DATA_SEG
    push    CODE_SEG

    jmp     KERNEL_ENTRY

    jmp $

DA_PACKET:
			db 0x10					; size of packet (16 bytes)
			db 0					; always 0
blkcnt: 	dw MAX_SECTORS			; num of sectors to transfer
low_tloc: 	dw 0			 		; transfer location (0xFFFF:0xFFFF)	
high_tloc:	dw 0x1000
low_lba: 	dd 0
high_lba:	dd 0


; used memory
BOOT_DRIVE:			db 0x80
loadedSuccessfull: 	db "Loaded", 0x0
errorCode:			db "Error", 0x0

times 510-($-$$) db 0
dw 0xaa55
