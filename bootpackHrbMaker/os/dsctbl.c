#include "bootpack.h"

void init_gdtidt()
{
      struct SEGMENT_DESC* gdt = (struct SEGMENT_DESC *)GDT_ADDR;
      struct GATE_DESC* idt = (struct GATE_DESC *) IDT_ADDR ;
      int i;
      for (i = 0; i <= LIMIT_GDT/8; i++) 
	set_segmdesc(gdt + i, 0, 0, 0);
	
      set_segmdesc(gdt + 1, 0xffffffff, 0, AR_DATA32_RW);
      set_segmdesc(gdt + 2, 0x7ffff, 0x280000, AR_CODE32_ER);
      load_gdtr(0xffff, GDT_ADDR);
	
      for(i=0; i<= LIMIT_IDT /8; i++)
	set_gatedesc(idt+i, 0, 0, 0);
      load_idtr(0x7ff, IDT_ADDR);

      //hardware interrupt
      //set_gatedesc(idt + 0x0c, (int) asm_inthandler0c, 2 * 8, AR_INTGATE32);
      set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 * 8, AR_INTGATE32);
      set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
      set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
      set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
      set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);

      //software interrupt
      set_gatedesc(idt + 0x40, (int) asm_os_api, 2 * 8, AR_INTGATE32 + APP_ATTR);

      return;
}

void set_segmdesc(struct SEGMENT_DESC* sd, unsigned int limit, int base, int ar)
{
      if(limit > 0xfffff)
      {
	ar |= 0x8000;
	limit /= 0x1000;
      }
      sd->limit_low = limit & 0xffff;
      sd->base_low = base & 0xffff;
      sd->base_mid = (base >> 16) & 0xff;
      sd->access_right = ar & 0xff;
      sd->limit_high = ((limit >> 16) & 0xf) | ((ar >> 8) & 0xf0);
      sd->base_high = (base >> 24) & 0xff;
      return ;
}

void set_gatedesc(struct GATE_DESC* gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
	return;
}
