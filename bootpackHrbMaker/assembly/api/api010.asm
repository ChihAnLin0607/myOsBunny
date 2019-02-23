[BITS 32]

GLOBAL  _api_free

[section .text]

_api_free:			; void api_free(char* addr, int size)
	push	ebx
	mov		edx, 10
	mov 	ebx, [cs:0x20]
	mov		eax, [esp+8]
	mov		ecx, [esp+12]
	int		0x40
	pop		ebx
	ret

