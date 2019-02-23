#include"bootpack.h"

struct FIFO32* keyfifo;
int keydata0;

void wait_KBC_sendready()
{
	while(1)
	{
		if(!(io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOT_READY))
			return;
	}
}

void init_keyboard(struct FIFO32* fifo, int data0)
{
	keyfifo = fifo;
	keydata0 = data0;

	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

void inthandler21(int *esp)	// Keyboard
{
	int data;
	io_out8(PIC_M_OCW2, 0x61);
	data =  io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, keydata0+data);	
	return;
}
