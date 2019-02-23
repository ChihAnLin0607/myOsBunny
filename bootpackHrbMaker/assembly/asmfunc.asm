[BITS 32]
GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt
GLOBAL _io_in8, _io_in16, _io_in32
GLOBAL _io_out8, _io_out16, _io_out32
GLOBAL _io_load_eflags, _io_store_eflags
GLOBAL _load_gdtr, _load_idtr
GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
GLOBAL _asm_inthandler20, _asm_inthandler0d, _asm_inthandler0c, _asm_end_app
GLOBAL _load_cr0, _store_cr0
GLOBAL _memtest_sub
GLOBAL _load_tr, _farjmp, _farcall
GLOBAL _asm_os_api, _start_app


EXTERN	_inthandler20, _inthandler21, _inthandler27, _inthandler2c, _inthandler0d, _inthandler0c
EXTERN	_os_api

[section .text]

_io_hlt:	; void io_hlt(void);	
	hlt
	ret

_io_cli:	;clear interrupt flag
	cli
	ret

_io_sti:	;set interrupt flag
	sti
	ret

_io_stihlt:
	sti
	hlt
	ret

_io_in8:	; io_in8(int port)
	mov 	edx, [esp+4]  ; port
	mov 	eax, 0
	in 		al, dx
	ret

_io_in16:	; io_in16(int port)
	mov 	edx, [esp+4]
	mov 	eax, 0
	in 		ax, dx
	ret

_io_in32:	; io_in32(int port)
	mov 	edx, [esp+4]
	in 		eax, dx
	ret

_io_out8:	;io_out8(int port, int data)
	mov 	edx, [esp+4]
	mov 	al, [esp+8]
	out		dx, al
	ret

_io_out16:	;io_out16(int port, int data)
	mov 	edx, [esp+4]
	mov 	eax, [esp+8]
	out		dx, ax
	ret

_io_out32:	;io_out32(int port, int data)
	mov 	edx, [esp+4]
	mov 	eax, [esp+8]
	out		dx, eax
	ret

_io_load_eflags:	;int io_load_eflags();
	pushfd
	pop 	eax
	ret

_io_store_eflags:	;void io_store_eflags(int eflags);
	mov 	eax, [esp+4]
	push 	eax
	popfd
	ret

_load_gdtr:
	mov 	ax, [esp+4]
	mov		[esp+6], ax
	LGDT	[esp+6]
	ret

_load_idtr:
	mov		ax, [esp+4]
	mov 	[esp+6], ax
	LIDT	[esp+6]
	ret

_asm_inthandler20:
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler20
	pop		eax
	popad
	pop		ds
	pop		es
	iretd
	
_asm_inthandler21:
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler21
	pop		eax
	popad
	pop		ds
	pop		es
	iretd
	
_asm_inthandler2c:
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler2c
	pop		eax
	popad
	pop		ds
	pop		es
	iretd
	
_asm_inthandler27:
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler27
	pop		eax
	popad	
	pop		ds
	pop		es
	iretd
	
_asm_inthandler0d:
	sti
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler0d
	cmp		eax, 0
	jne		_asm_end_app
	pop		eax
	popad
	pop		ds
	pop		es
	add		esp, 4
	iretd

_asm_inthandler0c:
	sti
	push	es
	push	ds
	pushad
	mov		eax, esp
	push	eax
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	call	_inthandler0c
	cmp		eax, 0
	jne		_asm_end_app
	pop		eax
	popad
	pop		ds
	pop		es
	add		esp, 4
	iretd

_load_cr0:
	mov		eax, CR0
	RET

_store_cr0:
	mov		eax, [esp+4]
	mov		CR0, eax
	RET

_memtest_sub:						; unsinged int mentest_sub(unsigned int start, unsigned int end)
	push	edi
	push	esi
	push	ebx
	mov		esi, 0xaa55aa55			; pat0 = 0xaa55aa55;
	mov		edi, 0x55aa55aa			; pat1 = 0x55aa55aa;
	mov		eax, [esp+12+4]			; i = start;
mts_loop:
	mov 	ebx, eax				; p = i;
	add		ebx, 0xffc				; p += 0xffc;
	mov		edx, [ebx]				; org = *p;
	mov		[ebx], esi				; *p = pat0;
	xor		dword [ebx], 0xffffffff	; *p ^= 0xffffffff;
	cmp		edi, [ebx]				; if(*p != pat1)
	jne		mts_fin						;     goto mts_fin;
	xor		dword [ebx], 0xffffffff	; *p ^= 0xffffffff;
	cmp		esi, [ebx]				; if(*p != pat0)
	jne		mts_fin					;	  goto fin;
	mov		[ebx], edx				; *p = org;
	add		eax, 0x1000				; i += 0x1000;
	cmp		eax, [esp+12+8]			; if(i <= end)
	jbe		mts_loop				;	  goto mst_loop;
	pop		ebx
	pop 	esi
	pop		edi
	ret

mts_fin:
	mov		[ebx], edx				; *p = org
	pop		ebx
	pop		esi
	pop		edi
	ret

_load_tr:	;void load_tr(int tr);
	LTR		[esp+4]
	ret

_farjmp:	;void farjmp(int eip, int cs);
	jmp		far [esp+4]
	ret

_farcall:	;void farcall(int eip, int cs);
	call	far [esp+4]
	ret

_asm_os_api:		; INT 40
	sti
	push	ds
	push	es
	pushad
	pushad
	mov		ax, ss
	mov		ds, ax
	mov		es, ax
	
	call	_os_api
	
	cmp		eax, 0
	jne		_asm_end_app
	add		esp, 32
	popad
	pop		es
	pop		ds
	iretd
_asm_end_app:
	mov		esp, [eax]
	mov		dword [eax+4], 0
	popad
	ret

_start_app:		;start_app(int eip, int cs, int esp, int ds, int* tss_esp0)
	pushad		;push eax, ecx, edx, ebx, esp, ebp, esi, edi
	mov		eax, [esp+36]	; eip
	mov		ecx, [esp+40]	; cs
	mov		edx, [esp+44]	; esp
	mov		ebx, [esp+48]	; ds/ss
	mov		ebp, [esp+52]	; tss_esp
	mov		[ebp], esp
	mov		[ebp+4], ss
	mov		es, bx
	mov		ds, bx
	mov		fs, bx
	mov		gs, bx

	; modify stack for "retf" to application
	or 		ecx, 3
	or		ebx, 3
	push	ebx
	push	edx
	push	ecx
	push	eax
	retf
