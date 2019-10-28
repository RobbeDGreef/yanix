[org 0x7c00]
;RAMDISK_SIZE    equ 0x19000 ; this is 100 kb
RAMDISK_SIZE    equ 0x10000 ; 0x14000
    mov     [BOOT_DRIVE], dl
    mov     bp, 0x8000
    mov     sp, bp
    
    call    load_kernel

    ; vesa graphics code (should be called in kernel but i'm retarded and this works so...)
    
    ; MODE    RESOLUTION  BITS PER PIXEL  MAXIMUM COLORS
    ; 0x0100  640x480     4               16
    ; 0x0101  640x480     8               256
    ; 0x0102  800x600     4               16
    ; 0x0103  800x600     8               256
    ; 0x010D  320x200     15              32k
    ; 0x010E  320x200     16              64k
    ; 0x010F  320x200     24/32*          16m
    ; 0x0110  640x480     15              32k
    ; 0x0111  640x480     16              64k
    ; 0x0112  640x480     24/32*          16m
    ; 0x0113  800x600     15              32k
    ; 0x0114  800x600     16              64k
    ; 0x0115  800x600     24/32*          16m
    ; 0x0116  1024x768    15              32k
    ; 0x0117  1024x768    16              64k
    ; 0x0118  1024x768    24/32*          16m

    mov     ax, 0x4f02
    mov     bx, 0x4118      ; in qemu 24 bit and LFB is 0xfd000000 (yes i know i should call 0xf01 but this works and it just is a hobby project)
    int     0x10
    cmp     ax, 0x004f
    jne     vesaerror

    call    switch_to_pm

vesaerror:
    mov     bx, VERROR
    call    print
    jmp $

%include "boot/print.asm"
%include "boot/32bitPrint.asm"
%include "boot/loadFromDisk.asm"

%include "boot/gdt/gdt.asm"
%include "boot/gdt/switch.asm"

[bits 16]
load_kernel:
    ; first load the ramdisk 
    ; we will use location 0x500 as a start for the ramdisk and we will load 20 sectors = 10 kb 
    ;mov     bx, 0x500
    ;mov     cl, 2
    ;mov     dh, 20
    ;call    load_disk

    ;mov     bx, loadedSuccessfull
    ;call    print
    ;call    print_newline


    ; AH = 02h
    ; AL = number of sectors to read (must be nonzero)
    ; CH = low eight bits of cylinder number
    ; CL = sector number 1-63 (bits 0-5)
    ; high two bits of cylinder (bits 6-7, hard disk only)
    ; DH = head number
    ; DL = drive number (bit 7 set for hard disk)
    ; ES:BX -> data buffe

    ; now load kernel to high address
    
    ;mov     bx, 0
    ;x:
    ;    hlt
    ;    cmp bx, 0xf
    ;    je y
    ;    inc bx;

    ;    loop x
    ;y:
    

    mov     bx, 0x1000
    mov     es, bx
    mov     bx, 0x0000 
    
    mov     ah, 0x02    ; instruction

    mov     al, 0x80    ; number of sectors to read must be nonzero (0x01 - 0x80) 128
    mov     cl, 0x02    ; sector number 1-63 bits (0-5) 
    mov     ch, 0x00    ; cylinder (0x0 - 0x3ff) 1023
    mov     dh, 0x00    ; head number (0x00 - 0x0f) (side i think)
    int     0x13

    jc disk_error
    cmp al, 128
    jne sectors_error

    mov     bx, loadedSuccessfull
    call    print

    mov     bx, 0x2000
    mov     es, bx
    mov     bx, 0x0000 
    
    mov     ah, 0x02    ; instruction

    mov     al, 0x80    ; number of sectors to read must be nonzero (0x01 - 0x80) 128
    mov     cl, 0x16    ; sector number 1-63 bits (0-5) 
    mov     ch, 0x01    ; cylinder (0x0 - 0x3ff) 1023
    mov     dh, 0x01    ; head number (0x00 - 0x0f) (side i think)
    int     0x13

    jc disk_error
    cmp al, 128
    jne sectors_error

    mov     bx, loadedSuccessfull
    call    print

    ret

[bits 32]
pm_begin:
    push    DATA_SEG
    push    CODE_SEG

    jmp     0x10000+RAMDISK_SIZE

    jmp $
    ;jmp     0x24000

SPACE db " ", 0x0
loadedSuccessfull db "loaded ", 0x0
VERROR db "vesa err", 0x0
BOOT_DRIVE db 0x80
times 510-($-$$) db 0
dw 0xaa55