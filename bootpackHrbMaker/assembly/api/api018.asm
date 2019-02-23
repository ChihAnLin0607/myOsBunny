[BITS 32]

GLOBAL _api_settimer

[section .text]

_api_settimer:		; void api_settimer(int timer, int time)
	push	ebx
	mov		edx, 18
	mov		ebx, [esp+8]
	mov		eax, [esp+12]
	int		0x40
	pop		ebx
	ret
