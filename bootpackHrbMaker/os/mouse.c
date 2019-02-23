#include"bootpack.h"

struct FIFO32* mousefifo;
int mousedata0;

void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DECODE* mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;

	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SEND_TO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSE_CMD_ENABLE);
	mdec->phase = 0;
	return;
}

int mouse_decode(struct MOUSE_DECODE* mdec, unsigned char data)
{
	switch(mdec->phase)
	{
	      case 0:
		if(data == 0xfa)
		      mdec->phase = 1;
		return 0;
	      case 1:
		if( (data & 0xc8) == 0x08 ) 	// data must be 00xx1xxx(or 0x08~0x3F)
		{
		      mdec->buf[0] = data;
		      mdec->phase = 2;
		}
		return 0;
	      case 2:
		mdec->buf[1] = data;
		mdec->phase = 3;
		return 0;
	      case 3:	
		mdec->buf[2] = data;
		mdec->phase = 1;
		mdec->low3 = mdec->buf[0] & 0x07;	// 0x07 = 0000 0111
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if( mdec->buf[0] & 0x10 )
		      mdec->x |= 0xffffff00;
		if( mdec->buf[0] & 0x20 )
		      mdec->y |= 0xffffff00;
		mdec->y = -mdec->y;
		return 1;
	}
	return -1;
}

void inthandler2c(int *esp)
{
	int data;
	io_out8(PIC_S_OCW2, 0x64);
	io_out8(PIC_M_OCW2, 0x62);	
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data+mousedata0);
	return;
}
