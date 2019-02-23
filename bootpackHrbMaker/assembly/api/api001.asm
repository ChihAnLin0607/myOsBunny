[BITS 32]

GLOBAL	_api_putchar

[section .text]

_api_putchar:
	mov edx, 1
	mov al, [esp+4]
	int 0x40
	ret
