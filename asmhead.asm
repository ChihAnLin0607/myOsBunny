VBEMODE	EQU		0x105

BOTPAK	EQU		0x00280000
DSKCAC	EQU		0x00100000
DSKCAC0	EQU		0x00008000

CYLS	EQU		0x0ff0
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2
SCRNX	EQU		0x0ff4
SCRNY	EQU		0x0ff6
VRAM	EQU		0x0ff8

		ORG		0xc400

;check whether VBE exist
		MOV		AX, 0x9000
		MOV		ES, AX
		MOV		DI, 0
		MOV		AX, 0x4f00
		INT		0x10
		CMP		AX, 0X004f
		JNE		scrn320		; no VBE
;get the version of VBE
		MOV		AX, [ES:DI+4]
		CMP		AX, 0x0200
		JB		scrn320		; VBE version< 2.0

; check whether VBMODE is usable
		MOV		CX, VBEMODE
		MOV		AX, 0x4f01
		INT		0x10
		CMP		AX, 0x004f
		JNE		scrn320

		CMP		BYTE [ES:DI+0x19], 8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b], 4
		JNE		scrn320
		MOV		AX, [ES:DI+0x00]
		AND		AX, 0x0080
		JZ		scrn320
; switch to VBEMODE and save relevance data to specific memory address
		MOV		BX, VBEMODE+0x4000
		MOV		AX, 0x4f02
		INT		0x10
		MOV		BYTE [VMODE], 8
		MOV		AX, [ES:DI+0x12]
		MOV		[SCRNX], AX
		MOV		AX, [ES:DI+0x14]
		MOV		[SCRNY], AX
		MOV		EAX, [ES:DI+0X28]
		MOV		[VRAM], EAX
		JMP		keystatus

;		MOV		BX, 0x4101
;		MOV		AX, 0x4f02
;		INT 	0x10
;		MOV		BYTE [VMODE], 8
;		MOV		WORD [SCRNX], 640
;		MOV		WORD [SCRNY], 480
;		MOV		DWORD [VRAM], 0xe0000000
;		JMP		keystatus

scrn320:
		MOV		AL, 0x13
		MOV		AH, 0x00
		INT		0x10
		MOV		BYTE [VMODE], 8	
		MOV		WORD [SCRNX], 320
		MOV		WORD [SCRNY], 200
		MOV		DWORD [VRAM], 0x000a0000

keystatus:
		MOV		AH, 0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

		;##########################################

		MOV		AL,0xff
		OUT		0x21,AL			; io_out(PIC_M_IMR, 0xff), block all IRQ to master PIC
		NOP	
		OUT		0xa1,AL			; io_out(PIC_S_IMR, 0xff), block all IRQ to slave PIC

		;##########################################

		CLI

		;#########################################

		CALL	waitkbdout		; wait_KBC_sendready();
		MOV		AL,0xd1			; io_out8(PORT_KEYCMD, KEYCMD_WRITE_OUTPUT);
		OUT		0x64,AL			; PORT_KEYCMD == 0x64, KEYCMD_WRITE_OUTPUT == 0xd1 
		CALL	waitkbdout		; wait_KBC_sendready();
		MOV		AL,0xdf			; io_out8(PORT_KEYDAT, KBC_OUTPUT_A20G_ENABLE); to enable A20
		OUT		0x60,AL			; PORT_KEYDAT == 0x60, KBC_OUTPUT_A20G_ENABLE == 0xdf
								; A20GATE is a special signal line to enable the memory above 1MB
		CALL	waitkbdout		; wait_KBC_sendready();

		
		LGDT	[GDTR0]
		MOV		EAX,CR0			; CR0 is Controller Register 0, setting the lowest bit of CR0 to 1 and the highest bit to 0 will
		AND		EAX,0x7fffffff	; change CPU mode to Potect mode. Protect Mode will use GDT and protect the os code from user application
		OR		EAX,0x00000001
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; initial segment registers
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

		MOV		ESI,bootpack	; copy bootpack to 0x280000, size = 512KB
		MOV		EDI,BOTPAK
		MOV		ECX,512*1024/4
		CALL	memcpy

		MOV		ESI,0x7c00		; copy ipl to 0x100000, size = 512 bytes
		MOV		EDI,DSKCAC
		MOV		ECX,512/4
		CALL	memcpy

		MOV		ESI,DSKCAC0+512	; copy the rest of the floppy disk to 0x100200
		MOV		EDI,DSKCAC+512
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4
		SUB		ECX,512/4
		CALL	memcpy

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip
		MOV		ESI,[EBX+20]
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout	
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy		
		RET

		ALIGNB	16
GDT0:
		RESB	8
		DW		0xffff,0x0000,0x9200,0x00cf
		DW		0xffff,0x0000,0x9a28,0x0047

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
