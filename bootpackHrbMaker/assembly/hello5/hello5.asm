[BITS 32]
GLOBAL	_HariMain
[Section .text]
_HariMain:
	mov		edx, 2
	mov		ebx, msg
	INT 	0x40
	mov		edx, 4
	INT		0x40

[Section .data]
msg:
	DB	"Hello, World", 0x0a, 0
