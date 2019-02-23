[BITS 32]

GLOBAL	_api_putstr_len

[section .text]

_api_putstr_len: ;void api_putstr_len(char* s, int l)
	push	ebx
	mov		edx, 3
	mov		ebx, [esp+8]
	mov		ecx, [esp+12]
	int		0x40
	pop		ebx
	ret
