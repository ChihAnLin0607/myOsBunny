[BITS 32]

GLOBAL _api_fclose

[section .text]

_api_fclose:			; void api_fclose(int fhandle)
	mov		edx, 22
	mov		eax, [esp+4]
	int		0x40
	ret
