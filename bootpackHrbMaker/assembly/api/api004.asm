[BITS 32]

GLOBAL	_api_end

[section .text]

_api_end:
	mov	edx, 4
	int 0x40

