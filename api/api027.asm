[BITS 32]

GLOBAL _api_getlang

[section .text]

_api_getlang:			; int api_getlang();
	mov		edx, 27
	int		0x40
	ret
