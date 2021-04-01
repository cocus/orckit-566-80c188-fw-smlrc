bits 16

    ;extern ___start__
	extern _main
    extern __start__bss
    extern __stop__bss

	extern __start__data
	extern __stop__data

	extern __start_alldata__
	extern __stop_alldata__	
	
	extern __stop_allcode__
	extern __start_allcode__
	
section .text

    global __start
__start:
	
    ;mov ax, ss
	mov ax, 0900h
	mov ss, ax
    mov ds, ax ; DS=ES=SS in small model .EXEs and in tiny model .COMs
    mov es, ax
    ; Init .bss
    mov di, __start__bss
    mov cx, __stop__bss
    sub cx, di
    xor al, al
    cld
    rep stosb

	
	mov ax, cs
	mov ds, ax
	
	mov ax, __stop_allcode__
	sub ax, __start_allcode__
	

	
	mov bx, ax
	and bx, 0fh
	test bl, bl
	jz no_fix
	sub ax, bx
	add ax, 010h
	
no_fix:
	mov si, 0000h
	
	add si, ax
	mov cx, __stop__data

	xor di, di

	shr     cx,1            ; carry flag has remainder after divide by 2
	jnc     cxeven            ; if no carry, we are finished
	movsb					; Copy a byte from DS:SI to ES:DI
cxeven:
	rep     movsw           ; move this many words
    mov ax, ss
	mov ss, ax
    mov ds, ax ; DS=ES=SS in small model .EXEs and in tiny model .COMs
    mov es, ax
    jmp _main ; __start__() will set up argc and argv for main() and call exit(main(argc, argv))

section .bss
    ; .bss must exist for __start__bss and __stop__bss to also exist
	
section .data

