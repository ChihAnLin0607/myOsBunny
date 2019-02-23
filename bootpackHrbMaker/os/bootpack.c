#include<stdio.h>
#include<string.h>
#include "bootpack.h"

extern struct TASKCTL* taskctl;

void keysht_off(struct SHEET* key_to_sht);
void keysht_on(struct SHEET* key_to_sht);
void close_constask(struct TASK* task);
void close_console(struct SHEET* sht);

void HariMain()
{
      struct BOOTINFO* bootinfo = (struct BOOTINFO*)BOOTINFO_ADDR;
      struct FIFO32 fifo, keycmd_fifo;
      char s[64];
      int fifobuf[128], keycmd_fifo_buf[32];
      int i, x, y, mmx = -1, mmy = -1, mmx2 = 0;		//mm == move mode
      int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
      struct MOUSE_DECODE mdec;
      struct MEMMAN *mem_manager = (struct MEMMAN*)MEMMAN_ADDR;
      struct SHTCTL* shtctl;
      struct SHEET* sht = 0, *key_to_sht, *sht2;
      int key_shift = 0, key_leds = (bootinfo->leds >> 4) & 7, keycmd_wait = -1;
      static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', ASCII_BACKSPACE,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', ASCII_RETURN,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
      };
      static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', ASCII_BACKSPACE, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', ASCII_RETURN,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
      };

      unsigned char *buf_back, buf_mouse[256];
      struct SHEET* sht_back, *sht_mouse;
      struct TASK* task_a, *task;

      init_gdtidt();
      init_pic();
      io_sti();

      fifo32_init(&fifo, 128, fifobuf, 0);
      fifo32_init(&keycmd_fifo, 32, keycmd_fifo_buf, 0);
      FIFO_ADDR = (int)&fifo;

      init_pit();
      init_keyboard(&fifo, KEYBOARD_DATA0);
      enable_mouse(&fifo, MOUSE_DATA0, &mdec);
      io_out8(PIC_M_IMR, 0xf8);
      io_out8(PIC_S_IMR, 0xef); 

      unsigned int mem_total = memtest(0x400000, 0xbfffffff);
      memman_init(mem_manager);
      memman_free(mem_manager, 0x1000, 0x9e000);
      memman_free(mem_manager, 0x400000, mem_total - 0x400000);

      init_palette();
      shtctl = shtctl_init(mem_manager, bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
      SHTCTL_ADDR = (int)shtctl;

      /****************** task_a ******************/
      task_a = task_init(mem_manager);
      task_a->langmode = ENG;
      fifo.task = task_a;
      task_run(task_a, 1, 0);

      /****************** nihongo ******************/
      int* fat;
      unsigned char* nihongo;
      struct FILE_INFO *finfo;
      extern char hankaku[4096];

      fat = (int*) memman_alloc_4k(mem_manager, 4*2880);
      file_readfat(fat, (unsigned char*)(DISKIMG_ADDR + FAT_OFFSET));
      finfo = file_search("nihongo.fnt", (struct FILE_INFO*)
      (DISKIMG_ADDR+DIR_TABLE_ADDR), DISK_MAX_FILES);
      if(finfo)
      {
	i = finfo->size;
	nihongo = file_loadfile2(finfo->clustno, &i, fat);
      }
      else
      {
	nihongo = (unsigned char*) memman_alloc_4k(mem_manager, 16*256 + 32*94*47);
	for(i=0; i<16*256; i++)
	      nihongo[i] = hankaku[i];
	for(i = 16*256; i < 16*256 + 32*94*47; i++)
	      nihongo[i] = 0xff;
      }
      NIHONGO_ADDR = (int)nihongo;
      memman_free_4k(mem_manager, (int)fat, 4*2880);
      
      /****************** sht_back ******************/
      sht_back = sheet_alloc(shtctl);
      buf_back = (unsigned char*)memman_alloc_4k(mem_manager, 
		bootinfo->scrnx * bootinfo->scrny);
      sheet_setbuf(sht_back, buf_back, bootinfo->scrnx, bootinfo->scrny, -1);
      init_scrn(bootinfo->scrnx, bootinfo->scrny, buf_back);     

      /****************** console ******************/
      key_to_sht = open_console(shtctl, mem_total);
      keysht_on(key_to_sht);

      /****************** sht_mouse ******************/
      sht_mouse = sheet_alloc(shtctl);
      sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
      init_mouse_cursor8(buf_mouse, 99);
      int mx = (bootinfo->scrnx - 16) / 2;
      int my = (bootinfo->scrny - 28 - 16) / 2;

      sheet_slide(sht_back, 0, 0);
      sheet_slide(key_to_sht, 32, 4);
      sheet_slide(sht_mouse, mx, my);
     
      sheet_updown(sht_back, 0);
      sheet_updown(key_to_sht, 1);
      sheet_updown(sht_mouse, 2);

      fifo32_put(&keycmd_fifo, KEYCMD_LED);
      fifo32_put(&keycmd_fifo, key_leds);

      int data;
      while(1)
      {
	if(fifo32_status(&keycmd_fifo) > 0 && keycmd_wait < 0)
	{
	      keycmd_wait = fifo32_get(&keycmd_fifo);
	      wait_KBC_sendready();
	      io_out8(PORT_KEYDAT, keycmd_wait);
	}
	io_cli();
	if(!fifo32_status(&fifo))
	{
	      if(new_mx > 0)
	      {
		io_sti();
		sheet_slide(sht_mouse, new_mx, new_my);
		new_mx = -1;
	      }
	      else if(new_wx != 0x7fffffff)
	      {
		io_sti();
		sheet_slide(sht, new_wx, new_wy);
		new_wx = 0x7fffffff;
	      }
	      else
	      {
		task_sleep(task_a);
		io_sti();
	      }
	}
	else
	{
	      data = fifo32_get(&fifo);
	      io_sti();
	      if(key_to_sht && !key_to_sht->flags)
	      {
		if(shtctl->top == 1)
		      key_to_sht = 0;
		else
		{
		      key_to_sht = shtctl->sheets[shtctl->top-1];
		      keysht_on(key_to_sht);
		}
	      }
	      /****************** Keyboard ******************/
	      if(KEYBOARD_DATA0 <= data && data <= KEYBOARD_DATA0+255)
	     {
		if(data < 0x80 + KEYBOARD_DATA0)
		{
		      if(!key_shift)
			s[0] = keytable0[data - KEYBOARD_DATA0];
		      else
			s[0] = keytable1[data - KEYBOARD_DATA0];
		}
		else
		      s[0] = 0;

		if('A' <= s[0] && s[0] <= 'Z')
		{
		      if( (!(key_leds & 4) && !key_shift ) ||
		          (  (key_leds & 4) &&  key_shift )  )
			s[0] += 0x20;
		}
		/*************** Normal Text¡BBackspace¡BEnter ***************/
		if(s[0] && key_to_sht)
		      fifo32_put(&key_to_sht->task->fifo, s[0]+KEYBOARD_DATA0);
		
		switch(data)
		{
		      /****************** Tab ******************/
		      case KEYBOARD_DATA0 + 0x0f:
			if(key_to_sht)
			{
			      keysht_off(key_to_sht);
			      int lower_sht_h = key_to_sht->height-1;
			      if(!lower_sht_h)
				lower_sht_h = shtctl->top-1;
			      key_to_sht = shtctl->sheets[lower_sht_h];
			      keysht_on(key_to_sht);
			}
			break;
		      /****************** Left Shift On ******************/
		      case KEYBOARD_DATA0 + 0x2a:
			key_shift |= 1;
			break;
		      /****************** Right Shift On ******************/
		      case KEYBOARD_DATA0 + 0x36:
			key_shift |= 2;
			break;
		      /****************** Left Shift Off ******************/
		      case KEYBOARD_DATA0 + 0xaa:
			key_shift &= ~1;
			break;
		      /****************** Right Shift Off ******************/
		      case KEYBOARD_DATA0 + 0xb6:
			key_shift &= ~2;
			break;
		      /******************** CapsLock ********************/
		      case KEYBOARD_DATA0 + 0x3a:
			key_leds ^= 4;
			fifo32_put(&keycmd_fifo, KEYCMD_LED);
			fifo32_put(&keycmd_fifo, key_leds);
			break;
		      /******************** NumLock ********************/
		      case KEYBOARD_DATA0 + 0x45:
			key_leds ^= 2;
			fifo32_put(&keycmd_fifo, KEYCMD_LED);
			fifo32_put(&keycmd_fifo, key_leds);
			break;
		      /******************** ScrollLock ********************/
		      case KEYBOARD_DATA0 + 0x46:
			key_leds ^= 1;
			fifo32_put(&keycmd_fifo, KEYCMD_LED);
			fifo32_put(&keycmd_fifo, key_leds);
			break;
		      /********** Send data to keyboard successfully **********/
		      case KEYBOARD_DATA0 + 0xfa:
			keycmd_wait = -1;
			break;
		      /************* Send data to keyboard fail *************/
		      case KEYBOARD_DATA0 + 0xfe:
			wait_KBC_sendready();
		 	io_out8(PORT_KEYDAT, keycmd_wait);
			break;
		      /********************* F11 *********************/
		      case KEYBOARD_DATA0 + 0x57:
			sheet_updown(shtctl->sheets[1], shtctl->top-1);
			break;
		}
		
		/************************ Shift + F1 *************************/
		if(data == KEYBOARD_DATA0 + 0x3b && key_shift && key_to_sht)
		{
		      task = key_to_sht->task;
		      if(task && task->tss.ss0)
		      {
			cons_putstr(task->cons, "\nBreak(key) :\n");
			io_cli();
			task->tss.eax = (int)&(task->tss.esp0);
			task->tss.eip = (int)asm_end_app;
			io_sti();
			task_run(task, TASK_LV, 0);	//priority 0, the highest one
		      }
		}
		/******************** Shift + F2 ********************/
		else if(data == KEYBOARD_DATA0 +  0x3c && key_shift)
		{
		      if(key_to_sht)
			keysht_off(key_to_sht);
		      key_to_sht = open_console(shtctl, mem_total);
		      sheet_slide(key_to_sht, 32, 4);
		      sheet_updown(key_to_sht, shtctl->top);
		      keysht_on(key_to_sht);
		}
	      }
	      /************************* Mouse *************************/ 
	      else if(MOUSE_DATA0 <= data && data <= MOUSE_DATA0+255)
	      {
		if(mouse_decode(&mdec, data-MOUSE_DATA0) == 1)
		{
		      mx += mdec.x;
		      my += mdec.y;
		      if(mx < 0 ) mx = 0;
		      if(my < 0 ) my = 0;
		      if(mx > bootinfo->scrnx - 1)
			mx = bootinfo->scrnx - 1;
		      if(my > bootinfo->scrny - 1)
			my = bootinfo->scrny - 1;

		      new_mx = mx;
		      new_my = my;
		      sheet_slide(sht_mouse, mx, my);
		       if(mdec.low3 & 0x1)	//left button click down
		      {
			if(mmx < 0)
			{
			      for(i = shtctl->top-1; i > 0; i--)
			      {
				sht = shtctl->sheets[i];
				x = mx - sht->vx0;
				y = my - sht->vy0;
				/**************** Click on the window ****************/
				if(0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize
				   && sht->buf[y*sht->bxsize + x] != sht->col_inv)
				{
				      sheet_updown(sht, shtctl->top-1);
				      if(sht != key_to_sht)
				      {
					keysht_off(key_to_sht);
					key_to_sht = sht;
					keysht_on(key_to_sht);
				      }
				      /******** Click on the title head of the window ********/
				      if(3 <= x && x < sht->bxsize -3 && 3 <= y && y < 21)
				      {
					mmx = mx;
					mmy = my;
					mmx2 = sht->vx0;
					new_wy = sht->vy0;
				      }
				      /******** Click on the X button of the window ********/
				      if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 
				         5 <= y && y < 19) 
				      {
					if(sht->flags & FLAG_APP_WIN)
					{
					      task = sht->task;
					      cons_putstr(task->cons, "\nBreak(mouse): \n");
					      io_cli();
					      task->tss.eax = (int) &(task->tss.esp0);
					      task->tss.eip = (int) asm_end_app;
					      io_sti();
					      task_run(task, TASK_LV, 0);//the highest priority
					}
					else	// console
					{
					      task = sht->task;
					      sheet_updown(sht, -1);
					      keysht_off(key_to_sht);
					      key_to_sht = shtctl->sheets[shtctl->top - 1];
					      keysht_on(key_to_sht);
					      io_cli();
					      fifo32_put(&task->fifo, CONS_CLOSE);
					      io_sti();
					}
				      }
				      break;
				}
			      }
			}
			else
			{
			      x = mx - mmx;
			      y = my - mmy;
			      new_wx = (mmx2 + x + 2) & 0xfffffffc;
			      new_wy = new_wy + y;
			      mmy = my;
			}
		      }
		      else		// else if(mdec.low3 & 0x1 == 0)
		      {
			mmx = -1;
			if(new_wx != 0x7fffffff)
			{
			      sheet_slide(sht, new_wx, new_wy);
			      new_wx = 0x7fffffff;
			}
		      }
		}
	      }
	      /********************* Terminal a console with sheet *********************/ 
	      else if(CONS_SHT_DATA0 <= data && data <= CONS_SHT_DATA0 + 255)
		close_console(shtctl->sheets0 + (data - CONS_SHT_DATA0));
	      /******************** Terminal a console without sheet ********************/ 
	      else if(CONS_NSHT_DATA0 <= data && data <= CONS_NSHT_DATA0 + 999)
		close_constask(taskctl->tasks0 + (data - CONS_NSHT_DATA0));
	      /************************* Free a console's sheet *************************/ 
	      else if(2024 <= data && data <= 2279)
	      {
		sht2 = shtctl->sheets0 + (data - 2024);
		memman_free_4k(mem_manager, (int)sht2->buf,
		(CONS_W+16)*(CONS_H+37));
		sheet_free(sht2);
	      }
	}
      }
}

void keysht_off(struct SHEET* key_to_sht)
{
      change_wtitle8(key_to_sht, 0);
      if(key_to_sht->flags & FLAG_WITH_CURSOR)
	fifo32_put(&key_to_sht->task->fifo, CURSOR_OFF);
      return;
}

void keysht_on(struct SHEET* key_to_sht)
{
      change_wtitle8(key_to_sht, 1);
      if(key_to_sht->flags & FLAG_WITH_CURSOR)
	fifo32_put(&key_to_sht->task->fifo, CURSOR_ON);
      return;
}

struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal)
{
      struct MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
      struct SHEET* sht = sheet_alloc(shtctl);
      unsigned char* buf = (unsigned char*)memman_alloc(memman, 
			 (CONS_W+16)*(CONS_H+37));
      sheet_setbuf(sht, buf, CONS_W+16, CONS_H+37, -1);
      make_window8(buf, CONS_W+16, CONS_H+37, "console", 0);
      make_textbox8(sht, CONS_X0, CONS_Y0, CONS_W, CONS_H, COL8_000000);
      sht->task = open_constask(sht, memtotal);
      sht->flags |= FLAG_WITH_CURSOR;
      return sht;
}

struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal)
{
      struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
      struct TASK* task = task_alloc();
      int* cons_fifo_buf = (int*) memman_alloc_4k(memman, 128*4);
      task->cons_stack = memman_alloc_4k(memman, 64*1024);
      task->tss.esp = task->cons_stack +64*1024-12;
      task->tss.eip = (int) &console_task;
      task->tss.es = 1*8;
      task->tss.cs = 2*8;
      task->tss.ss = 1*8;
      task->tss.ds = 1*8;
      task->tss.fs = 1*8;
      task->tss.gs = 1*8;
      *((int*)(task->tss.esp+4)) = (int)sht;
      *((int*)(task->tss.esp+8)) = memtotal;
      task_run(task, 2, 2);
      fifo32_init(&task->fifo, 128, cons_fifo_buf, task);
      return task;
}

void close_constask(struct TASK* task)
{
      struct  MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
      task_sleep(task);
      memman_free_4k(memman, task->cons_stack, 64 * 1024);
      memman_free_4k(memman, (int)task->fifo.buf, 128*4);
      task->flags = 0;
      return;
}

void close_console(struct SHEET* sht)
{
      struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
      struct TASK* task = sht->task;
      memman_free_4k(memman, (int)sht->buf, 256*165);
      sheet_free(sht);
      close_constask(task);
      return;
}
