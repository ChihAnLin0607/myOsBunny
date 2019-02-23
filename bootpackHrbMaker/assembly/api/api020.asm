[BITS 32]

GLOBAL _api_beep

[section .text]

_api_beep:			; void api_beep(int tone)
	mov		edx, 20
	mov		eax, [esp+4]
	int		0x40
	ret
