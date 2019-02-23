[BITS 32]

GLOBAL _api_alloctimer

[section .text]

_api_alloctimer:	; int api_alloctimer();
	mov		edx, 16
	int		0x40
	ret
