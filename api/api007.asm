[BITS 32]

GLOBAL _api_boxfillwin

[section .text]

_api_boxfillwin:	; void api_boxfillwin(int win, int x0, int y0, int x1, int y1, int color);
	push 	edi
	push	esi
	push	ebp
	push	ebx
	mov		edx, 7
	mov		ebx, [esp+20]	; win
	mov		eax, [esp+24]	; x0
	mov		ecx, [esp+28]	; y0
	mov		esi, [esp+32]	; x1
	mov		edi, [esp+36]	; y1
	mov		ebp, [esp+40]	; color
	int		0x40
	pop		ebx
	pop		ebp
	pop		esi
	pop		edi
	ret
