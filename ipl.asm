CYLS equ 20
		ORG		0x7c00
		jmp		entry

		db		0x90
		db		"HARIBOTE"
		dw		512
		db		1
		dw		1
		db		2
		dw		224
		dw		2880
		db		0xf0
		dw		9
		dw		18
		dw		2
		dd		0
		dd		2880
		db		0, 0, 0x29
		dd		0xffffffff
		db		"HARIBOTEOS "
		db		"FAT12   "
		RESB	18

entry:
		;initialize registers
		mov		ax, 0
		mov		ss, ax
		mov 	sp, 0x7c00
		mov		ds, ax
		
		mov		ax, 0x0820
		mov 	es, ax		; load data to address [es:bx] in memory
		mov		ch, 0		; C0
		mov		dh, 0		; H0
		mov		cl, 2		; S2

readloop:
		mov 	si, 0		; counter for the failure

retry:
		mov		ah, 0x02	; 2 for read
		mov		al, 1		; 1 sector
		mov		bx, 0		
		mov		dl, 0x00	; disk no.0
		int 	0x13
		jnc		next
		add		si, 1
		cmp		si, 5
		jae		error		; jump if above or equal

		mov		ah, 0x00	; 0 for reset the disk
		mov		dl, 0x00	; disk no.0
		int		0x13;
		jmp		retry;
next:
		mov		ax, es
		add		ax, 0x0020
		mov		es, ax
		add		cl, 1
		cmp		cl, 18
		jbe		readloop
		
		mov 	cl, 1		; reset the number of the sector
		add		dh, 1		; next head
		cmp 	dh, 2
		jb		readloop

		mov		dh, 0		; reset the number of the head
		add		ch, 1		; next cylinder
		cmp		ch, CYLS
		jb		readloop

		mov		[0x0ff0], ch; record how many cylinders have been read	
		jmp		0xc400		; ?0xa609 0xa610~ 0xc400

fin:
		hlt
		jmp		fin

error:
		mov		si, msg

putloop:
		mov		al, [si]
		add		si, 1
		cmp		al, 0
		je		fin
		mov		ah, 0x0e
		mov     bx, 15
		int		0x10
		jmp		putloop

msg:
		db		0x0a, 0x0a
		db		"load error"
		db		0x0a
		db		0

		RESB	0x7dfe - 0x7c00 - ($-$$)
		db		0x55, 0xaa
