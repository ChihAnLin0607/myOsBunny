[BITS 32]

GLOBAL _api_freetimer

[section .text]

_api_freetimer:		; void api_freetimer(int timer)
	push	ebx
	mov		edx, 19
	mov		ebx, [esp+8] 	; timer
	int 	0x40
	pop		ebx
	ret
