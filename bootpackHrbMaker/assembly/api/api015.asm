[BITS 32]

GLOBAL	_api_getkey

[section .text]

_api_getkey:		; int api_getkey(int mode);
	mov		edx, 15
	mov 	eax, [esp+4]
	int		0x40
	ret
