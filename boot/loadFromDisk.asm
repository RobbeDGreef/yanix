;load_disk:
    ;pusha
    ;push dx

    ;mov ah, 0x02
    ;mov al, dh
    ;mov cl, 0x02

    ;mov ch, 0x00
    ;mov dh, 0x00 

    ;int 0x13
    ;jc disk_error 

    ;pop dx
    ;cmp al, dh
    ;jne sectors_error
    ;popa
    ;ret

    ;popa
    ;ret

    ;pusha
    ;push dx
    ;mov al, dh
    ;mov ch, 0x00
    
    ;mov dh, 0x00

    ;mov     bx, 0x1000
    ;mov     es, bx
    ;mov     bx, 0x0000 
    ;mov     dh, 0x00    ; head number (0x00 - 0x0f)
    ;mov     cl, 0x02    ; sector on track (0x01 - 0x11) 17
    
    ;mov     ah, 0x02    ; instruction read
    ;mov     al, 128     ; amount of sectors from start point (0x01 - 0x80) 128
    ;mov     ch, 0x00    ; cylinder (0x0 - 0x3ff) 1023
    
    ;mov     bx, 0x1000
    ;mov     es, bx
    ;mov     bx, 0x0000 
    ;mov     al, 128     ; amount of sectors from start point (0x01 - 0x80) 128
    ;mov     dh, 0x00    ; head number (0x00 - 0x0f)
    ;mov     cl, 0x02    ; sector on track (0x01 - 0x11) 17
    ;mov     ch, 0x00    ; cylinder (0x0 - 0x3ff) 1023
    ;mov     ah, 0x02    ; instruction

    ;int     0x13
    ;jc disk_error
    ;cmp al, 128
    ;jne sectors_error

    ;mov     bx, 0x2000
    ;mov     es, bx
    ;mov     bx, 0x0000
    ;mov     al, 128
    ;mov     dh, 0x01
    ;mov     cl, 0x01
    ;mov     ch, 0x00
    ;mov     ah, 0x02
    ;int     0x13

    ;jc disk_error
    ;cmp al, 128
    ;jne sectors_error

    ;mov     bx, 0x3000
    ;mov     es, bx
    ;mov     bx, 0x0000
    ;mov     al, 128
    ;mov     dh, 0x02
    ;mov     cl, 0x01
    ;mov     ch, 0x00
    ;mov     ah, 0x02
    ;int     0x13

    ;jc disk_error
    ;cmp al, 128
    ;jne sectors_error

    ;jc disk_error
    ;pop dx
    ;cmp al, dh
    ;jne sectors_error

    ;popa
    ;ret

load_disk_cyl:
    ; AH = 02h
    ; AL = number of sectors to read (must be nonzero)
    ; CH = low eight bits of cylinder number
    ; CL = sector number 1-63 (bits 0-5)
    ; high two bits of cylinder (bits 6-7, hard disk only)
    ; DH = head number
    ; DL = drive number (bit 7 set for hard disk)
    ; ES:BX -> data buffer

    pusha

    ; set right registers
    mov     ah, 0x02
    mov     al, 0x24    ; read all of the 36 sectors
    mov     cl, 0x01
                        ; ch is set
    mov     dh, 0x00    ; for now we only use dh
    mov     dl, 0x00
    
    ; call the bios
    int     0x13        ; make the call
    
    ; check for errors
    jc      disk_error  ; we have an error if the carry is et
    cmp     al, 0x24
    jne     sectors_error

    popa
    ret

disk_error:
    mov bx, DISK_ERROR
    call print
    
    mov dh, ah
    call print_hex
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR
    call print

disk_loop:
    jmp $

DISK_ERROR: db "Disk read error", 0x00
SECTORS_ERROR: db "Incorrect number of sectors read", 0x00