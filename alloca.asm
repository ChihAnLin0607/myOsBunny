[BITS 32]
GLOBAL	__alloca

[section .text]
__alloca:
	add		eax, -4
	sub		esp, eax
	jmp		dword[esp+eax]
