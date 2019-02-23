[BITS 32]

GLOBAL _api_inittimer

[section .text]

_api_inittimer:		; void api_inittimer(int timer, int data)
	push	ebx
	mov		edx, 17
	mov		ebx, [esp+8]	; timer
	mov		eax, [esp+12]	; data
	int		0x40
	pop		ebx
	ret
