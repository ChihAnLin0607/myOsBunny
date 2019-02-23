[BITS 32]

GLOBAL _api_initmalloc

[section .text]

_api_initmalloc:	;void api_initmalloc();
	push	ebx
	mov		edx, 8
	mov		ebx, [cs:0x0020]
	mov		eax, ebx
	add		eax, 32*1024		; 32KB == sizeof(struct MEMMAN)
	mov		ecx, [cs:0x0000]
	sub		ecx, eax
	int 	0x40
	pop		ebx
