[BITS 32]

GLOBAL _api_putstrwin

[section .text]

_api_putstrwin:		; void api_putstrwin(int win, int x, int y, int col, int len, char* str);
	push	edi
	push	esi
	push	ebp
	push	ebx
	mov		edx, 6
	mov		ebx, [esp+20]	; win
	mov		esi, [esp+24]	; x
	mov		edi, [esp+28]	; y
	mov		eax, [esp+32]	; color
	mov		ecx, [esp+36]	; len
	mov		ebp, [esp+40]	; str
	int 	0x40
	pop		ebx
	pop		ebp
	pop		esi
	pop		edi
	ret

