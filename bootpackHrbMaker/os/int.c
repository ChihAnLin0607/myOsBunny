#include <stdio.h>
#include "bootpack.h"

void init_pic()
{
	io_out8(PIC_M_IMR,  0xff); 
	io_out8(PIC_S_IMR,  0xff); 

	io_out8(PIC_M_ICW1, 0x11);
	io_out8(PIC_M_ICW2, 0x20);
	io_out8(PIC_M_ICW3, 1 << 2);
	io_out8(PIC_M_ICW4, 0x01  );

	io_out8(PIC_S_ICW1, 0x11);
	io_out8(PIC_S_ICW2, 0x28);
	io_out8(PIC_S_ICW3, 2 ); 
	io_out8(PIC_S_ICW4, 0x01);

	io_out8(PIC_M_IMR,  0xfb);
	io_out8(PIC_S_IMR,  0xff);

	return;
}


void inthandler27(int *esp)
{
	io_out8(PIC_M_OCW2, 0x67);
	return;
}





