[BITS 32]

GLOBAL	_api_putstr

[section .text]

_api_putstr: ;void api_putstr(char* s)
	push	ebx
	mov		edx, 2
	mov		ebx, [esp+8]
	int		0x40
	pop		ebx
	ret
