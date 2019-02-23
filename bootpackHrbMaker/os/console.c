#include<stdio.h>
#include<string.h>
#include "bootpack.h"

extern struct TASKCTL* taskctl;

void console_task(struct SHEET* sheet, unsigned int memtotal)
{
      struct TASK* task = task_now();
      struct MEMMAN *memman = (struct MEMMAN*)MEMMAN_ADDR;
      int data, i;
      int *fat = (int*) memman_alloc_4k(memman, 4*2880);
      unsigned char* nihongo = (char*)NIHONGO_ADDR;
      char cmdline[128];

      if(nihongo[4096] != 0xff)
	task->langmode = JAN;
      else
	task->langmode = ENG;
      task->langbyte1 = 0;

      /****************** file handle ******************/
      struct FILEHANDLE fhandle[8];
      for(i = 0; i < 8; i++)
	fhandle[i].buf = 0;
      task->fhandle = fhandle;
      task->fat = fat;

      /****************** cons ******************/
      struct CONSOLE cons;
      cons.sht = sheet;
      cons.cur_x = CONS_X0;
      cons.cur_y = CONS_Y0;
      cons.cur_c = -1;

      task->cons = &cons;
      task->cmdline = cmdline;

      if(cons.sht)
      {
	cons.timer = timer_alloc();
	timer_init(cons.timer, &task->fifo, 1);
	timer_settime(cons.timer, 50);
      }

      file_readfat(fat, (unsigned char*)(DISKIMG_ADDR + FAT_OFFSET));

      cons_putchar(&cons, '>', 1);

      while(1)
      {
	io_cli();
	if(!fifo32_status(&task->fifo))
	{
	      task_sleep(task);
	      io_sti();
	}
	else
	{
	      data = fifo32_get(&task->fifo);
	      io_sti();
	      if(data <= 1 && cons.sht)		//timer for cursor
	      {
		if(data)
		{
		      timer_init(cons.timer, &task->fifo, 0);
		      if(cons.cur_c >= 0)
			cons.cur_c = COL8_FFFFFF;
		}
		else
		{
		      timer_init(cons.timer, &task->fifo, 1);
		      if(cons.cur_c >= 0)
			cons.cur_c = COL8_000000;
		}
		timer_settime(cons.timer, 50);
	      }
	      else if(data == CURSOR_ON)    // data == 2
		cons.cur_c = COL8_FFFFFF;
	      else if(data == CURSOR_OFF)  // data == 3
	      {
		if(cons.sht)
		{
		      boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000, cons.cur_x, 
		      cons.cur_y, cons.cur_x+7, cons.cur_y+15);
		}
		cons.cur_c = -1;
	      }
	      else if(data == CONS_CLOSE) // data == 4
		cmd_exit(&cons, fat);
	      else if(KEYBOARD_DATA0 <= data && data <= KEYBOARD_DATA0+255)
	      {
		switch(data)
		{
		      case KEYBOARD_DATA0 + ASCII_BACKSPACE:
			if(cons.cur_x > CONS_X0+CHAR_WIDTH)
			{
		 	      cons_putchar(&cons, ' ', 0);
			      cons.cur_x -= CHAR_WIDTH;
			}
			break;
		      case KEYBOARD_DATA0 + ASCII_RETURN:
			cons_putchar(&cons, ' ', 0);
			cmdline[cons.cur_x/CHAR_WIDTH - 2] = 0;
			cons_newline(&cons);
			cons_runcmd(cmdline, &cons, fat, memtotal);
			if(!cons.sht)
			      cmd_exit(&cons, fat);
			cons_putchar(&cons, '>', 1);
			break;
		      default:
			if(cons.cur_x < CONS_W)
			{
			      cmdline[cons.cur_x/CHAR_WIDTH-2]= data-						      KEYBOARD_DATA0;
			      cons_putchar(&cons, data-KEYBOARD_DATA0, 1);
			}
			break;
		}	
	      }
	      if(cons.sht)
	      {
		if(cons.cur_c >= 0)
		{
		      boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, 		      cons.cur_x+7, cons.cur_y + 15);
		}
		sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x+CHAR_WIDTH,
		cons.cur_y+16);
	      }
	}
      }
}

void cons_putchar(struct CONSOLE* cons, int chr, char move)
{
      char s[2];
      s[0] = chr;
      s[1] = 0;

      if(s[0] == 0x09)	//tab
      {
	while(1)
	{
	      if(cons->sht)
	      {
		putstring8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, 
		COL8_000000, " ", 1);
	      }
	      cons->cur_x += CHAR_WIDTH;
	      if(cons->cur_x == CONS_X0 + CONS_W)
		cons_newline(cons);
	      if(!((cons->cur_x - 8) & 0x1f))	//32n
		break;
	}
      }
      else if(s[0] == ASCII_RETURN)
	cons_newline(cons);
      else if(s[0] == 0x0d);
      else
      {
	if(cons->sht)
	{
	      putstring8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, 
	      COL8_000000, s, 1);
	}
	if(move)
	{
	      cons->cur_x += CHAR_WIDTH;
	      if(cons->cur_x == CONS_X0+CONS_W)
		cons_newline(cons);
	}
      }
      return;
}

void cons_newline(struct CONSOLE* cons)
{
      int x, y;
      struct SHEET* sheet = cons->sht;
      struct TASK* task = task_now();
      if(cons->cur_y < CONS_Y0 +CONS_H-16)
	cons->cur_y += 16;
      else
      {
	if(sheet)
	{
	      for(y = CONS_Y0; y<CONS_Y0+CONS_H-16 ; y++)
		for(x=CONS_X0; x < CONS_X0+CONS_W; x++)
		      sheet->buf[x+y*sheet->bxsize] = sheet->buf[x+(y+16)*sheet->bxsize];
	      for(y = CONS_Y0+CONS_H-16; y< CONS_Y0+CONS_H; y++)
		for(x=CONS_X0; x<CONS_X0+CONS_W; x++)
		      sheet->buf[x+y*sheet->bxsize] = COL8_000000;
	      sheet_refresh(sheet, CONS_X0, CONS_Y0, CONS_X0+CONS_W,
	      CONS_Y0+CONS_H);
	}
      }
      cons->cur_x = CONS_X0;
      if(task->langmode == JAN && task->langbyte1)
	cons->cur_x += CHAR_WIDTH;
      return;
}

void cons_putstr(struct CONSOLE* cons, char* str)
{
      for(; *str; str++)
	cons_putchar(cons, *str, 1);
      return;
}

void cons_putstr_len(struct CONSOLE* cons, char* str, int len)
{
      int i;
      for(i=0; i<len; i++)
	cons_putchar(cons, str[i], 1);
      return;
}

void cons_runcmd(char* cmdline, struct CONSOLE* cons, int *fat, unsigned int memtotal)
{
      if(!strcmp(cmdline, "mem"))
	cmd_mem(cons, memtotal);
      else if(!strcmp(cmdline, "clear"))
	cmd_clear(cons);
      else if(!strcmp(cmdline, "ls"))
	cmd_dir(cons);
      else if(!strcmp(cmdline, "exit"))
	cmd_exit(cons, fat);
      else if(!strncmp(cmdline, "start ", 6))
	cmd_start(cons, cmdline, memtotal);
      else if(!strncmp(cmdline, "ncst ", 5))
	cmd_ncst(cons, cmdline, memtotal);
      else if(!strncmp(cmdline, "langmode ", 9))
	cmd_langmode(cons, cmdline);
      else if(cmdline[0])
      {
	if(!cmd_app(cons, fat, cmdline))
	      cons_putstr(cons, "Bad Command.\n\n");
      }
      return;
}

void cmd_langmode(struct CONSOLE* cons, char* cmdline)
{
      struct TASK *task = task_now();
      unsigned char mode = cmdline[9] - '0';
      if(mode == ENG || mode == JAN || mode == ECU)
	task->langmode = mode;
      else
	cons_putstr(cons, "mode number error.\n");
      cons_newline(cons);
      return;
}

void cmd_ncst(struct CONSOLE* cons, char* cmdline, int memtotal)
{
      struct TASK* task = open_constask(0, memtotal);
      struct FIFO32* fifo = &task->fifo;
      int i;
      for(i=5; cmdline[i]; i++)
	fifo32_put(fifo, cmdline[i] + KEYBOARD_DATA0);
      fifo32_put(fifo, ASCII_RETURN + KEYBOARD_DATA0);
      cons_newline(cons);
      return;
}

void cmd_start(struct CONSOLE* cons, char* cmdline, int memtotal)
{
      struct SHTCTL* shtctl = (struct SHTCTL*)SHTCTL_ADDR;
      struct SHEET* sht = open_console(shtctl, memtotal);
      struct FIFO32 *fifo = &sht->task->fifo;
      int i;
      sheet_slide(sht, 32, 4);
      sheet_updown(sht, shtctl->top);
      for(i = 6; cmdline[i]; i++)
	fifo32_put(fifo, cmdline[i] + KEYBOARD_DATA0);
      fifo32_put(fifo, ASCII_RETURN + KEYBOARD_DATA0);
      cons_newline(cons);
      return;
}

void cmd_exit(struct CONSOLE* cons, int *fat)
{
      struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
      struct TASK* task = task_now();
      struct SHTCTL* shtctl = (struct SHTCTL*)SHTCTL_ADDR;
      struct FIFO32* fifo = (struct FIFO32*)FIFO_ADDR;
      timer_cancel(cons->timer);
      memman_free_4k(memman, (int)fat, 4*2880);
      io_cli();
      if(cons->sht)
	fifo32_put(fifo, cons->sht - shtctl->sheets0 + CONS_SHT_DATA0);
      else
	fifo32_put(fifo, task - taskctl->tasks0 + CONS_NSHT_DATA0);
      io_sti();
      while(1)
	task_sleep(task);
}

void cmd_mem(struct CONSOLE* cons, unsigned int memtotal)
{
      struct MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
      char s[30];	      
      sprintf(s,"total %dMB\nfree %dKB\n\n", memtotal/(1024*1024), 
      memman_total(memman)/1024);
      cons_putstr(cons, s);
      return;
}

void cmd_clear(struct CONSOLE* cons)
{
      int x, y;
      struct SHEET* sheet = cons->sht;
      for(y = CONS_Y0; y<CONS_Y0+CONS_H; y++)
	for(x=CONS_X0; x<CONS_X0+CONS_W; x++)
	      sheet->buf[x+y*sheet->bxsize] = COL8_000000;
      sheet_refresh(sheet, CONS_X0, CONS_Y0, CONS_X0+CONS_W, CONS_Y0+CONS_H);
      cons->cur_y = CONS_Y0;
      return;
}

void cmd_dir(struct CONSOLE* cons)
{
      struct FILE_INFO* file_info = 
      (struct FILE_INFO*)(DISKIMG_ADDR+DIR_TABLE_ADDR);
      int i, j;
      char s[30];
      for(i=0; i<DISK_MAX_FILES; i++)
      {
	if(file_info[i].name[0] == 0x00)
	      break;
	else if(file_info[i].name[0] != 0xe5)
	{
	      if(!(file_info[i].type & 0x18))
	      {
		sprintf(s, "filename.ext: %7d\n", file_info[i].size);
		for(j=0; j<8; j++)
		      s[j] = file_info[i].name[j];
		s[9] = file_info[i].ext[0];
		s[10] = file_info[i].ext[1];
		s[11] = file_info[i].ext[2];
		cons_putstr(cons, s);
	      }
	}
      }
      cons_newline(cons);
      return;
}

int cmd_app(struct CONSOLE* cons, int *fat, char* cmdline)
{
      struct SHTCTL* shtctl;
      struct SHEET* sht;

      struct MEMMAN *memman = (struct MEMMAN*)MEMMAN_ADDR;
      struct FILE_INFO* file_info;
      struct FILE_INFO* file_info_head = (struct FILE_INFO*)(DISKIMG_ADDR + 
      DIR_TABLE_ADDR);
      char name[18], *code_buf, *data_buf;
      struct TASK* task = task_now();

      int i;
      char is_ext_exist = 0;
      for(i=0; i<13; i++)
      {
	if(cmdline[i] <= ' ')
	      break;
	name[i] = cmdline[i];
	if(name[i] == '.')
	      is_ext_exist = 1;
      }
      name[i] = 0;

      file_info = file_search(name, file_info_head, DISK_MAX_FILES);
      if(!file_info && !is_ext_exist && i < 14)
      {
	name[i] = '.';
	name[i+1] = 'H';
	name[i+2] = 'R';
	name[i+3] = 'B';
	name[i+4] = 0;
	file_info = file_search(name, file_info_head, DISK_MAX_FILES);
      }

      if(file_info)
      {
	int segsize, data_size, esp, hrb_data, appsize;
	appsize = file_info->size;
	code_buf = file_loadfile2(file_info->clustno, &appsize, fat);
	if (appsize >= 36 && !strncmp(code_buf + 4, "Hari", 4) && *code_buf == 0x00)
	{
	      segsize = *((int*)(code_buf + 0x0000));
	      esp = *((int*)(code_buf + 0x000c));
	      data_size = *((int*)(code_buf + 0x0010));
	      hrb_data = *((int*)(code_buf + 0x0014));
	      data_buf = (char*)memman_alloc_4k(memman, segsize);
	      task->ds_base = (int)data_buf;

	      set_segmdesc(task->ldt, file_info->size-1, (int)code_buf,
	      AR_CODE32_ER+APP_ATTR);
	      set_segmdesc(task->ldt + 1, segsize-1, (int)data_buf, 
	      AR_DATA32_RW+APP_ATTR);

	      for(i=0; i<data_size; i++)
		data_buf[esp+i] = code_buf[hrb_data + i];
	      start_app(0x1b, 0*8 + LDT_FLAG, esp, 1*8 + LDT_FLAG, &(task->tss.esp0));

	      shtctl  = (struct SHTCTL*)SHTCTL_ADDR;
	      for(i = 0; i < MAX_SHEETS; i++)
	      {
		sht = &(shtctl->sheets0[i]);
		if( (sht->flags & SHEET_USED) && (sht->flags & FLAG_APP_WIN) 
		     && sht->task == task)
		      sheet_free(sht);
	      }
	      timer_cancelall(&task->fifo);
	      memman_free_4k(memman, (int)data_buf, segsize);
	      task->langbyte1 = 0;
	      for(i = 0; i < 8; i++)
	      {
		if(task->fhandle[i].buf)
		{
		      memman_free_4k(memman, (int)task->fhandle[i].buf, task->fhandle[i].size);
		      task->fhandle[i].buf = 0;
		}
	      }
	}
	else
	      cons_putstr(cons, ".hrb file format error.\n");
	memman_free_4k(memman, (int)code_buf, appsize);
	cons_newline(cons);
	return 1;
      }
      return 0;
}

int* os_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
      struct TASK* task = task_now();
      int ds_base = task->ds_base;
      struct CONSOLE* cons = task->cons;
      struct SHTCTL* shtctl = (struct SHTCTL*)SHTCTL_ADDR;
      struct FIFO32* sys_fifo = (struct FIFO32*)FIFO_ADDR;
      struct SHEET* sht;
      struct FILE_INFO* finfo;
      struct FILEHANDLE* fh;
      struct MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
      int* ret_addr = &eax + 8;	// *reg == &eax+1;  
      /*********************** PUSHAD ***********************/
      /* 	reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP	     */
      /*   reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
      /******************************************************/
      
      int data, i;
      switch(edx)
      {
	case 1:
	      cons_putchar(cons, eax & 0xff, 1);
	      break;
	case 2:
	      cons_putstr(cons, ds_base + (char*)ebx);
	      break;
	case 3:
	      cons_putstr_len(cons, ds_base + (char*)ebx, ecx);
	      break;
	case 4:
	      return &(task->tss.esp0);

	case 5:
	/***************  Show a new window ***************/
	/*		ebx: (char*) window's buffer		*/
	/*		esi: (int) xsize				*/
	/*		edi: (int) ysize				*/
	/*		eax: (int) color_inv			*/
	/*		ecx: (char*) title				*/
	/************************************************/
	      sht = sheet_alloc(shtctl);
	      sht->task = task;
	      sht->flags |= FLAG_APP_WIN;
	      sheet_setbuf(sht, (char*)ebx + ds_base, esi, edi, eax);
	      make_window8((char*)ebx + ds_base, esi, edi, (char*)ecx + ds_base, 0);
	      sheet_slide(sht, ((shtctl->xsize-esi)/2) & 0xfffffffc, ((shtctl->ysize-edi)/2)&0xfffffffc);
	      sheet_updown(sht, shtctl->top);
	      *ret_addr = (int)sht;
	      break;

	case 6:
	/************ Show the text in the window ************/
	/*		ebx: (int) window's number		*/
	/*		esi: (int) x				*/
	/*		edi: (int) y				*/
	/*		eax: (int) color				*/
	/*		ecx: (int) string length			*/
	/*		ebp: (char*) string			*/
	/************************************************/
	      sht = (struct SHEET*)(ebx & 0xfffffffe);
	      putstring8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char*) ebp + ds_base);
	      if(!(ebx & 1))
		sheet_refresh(sht, esi, edi, esi + ecx * CHAR_WIDTH, edi + 16);
	      break;

	case 7:
	/************ Show the box in the window ************/
	/*		ebx: (int) window's number		*/
	/*		eax: (int) x0				*/
	/*		ecx: (int) y0				*/
	/*		esi: (int) x1				*/
	/*		edi: (int) y1				*/
	/*		ebp: (int) color				*/
	/************************************************/
	      sht = (struct SHEET*)( ebx & 0xfffffffe);
	      boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
	      if(!(ebx & 1))
		sheet_refresh(sht, eax, ecx, esi+1, edi+1);
	      break;

	case 8:
	/******************* Init malloc *******************/
	/*		ebx: address of memman 		*/
	/*		eax: base address of the memory		*/ 
	/*		       managered by the memman		*/
	/*		ecx: the number of the bytes managered	*/ 
	/*		       by the mamman			*/
	/************************************************/
	      memman_init((struct MEMMAN*)(ebx + ds_base));
	      ecx &= 0xfffffff0;
	      memman_free((struct MEMMAN*)(ebx+ds_base), eax, ecx);
	      break;

	case 9:
	/********************* malloc *********************/
	/*		ebx: address of memman 		*/
	/*		ecx: the number of bytes needed		*/
	/*		eax: the address of the memory have been	*/ 
	/*		       located				*/
	/************************************************/
	      ecx = (ecx + 0x0f) & 0xfffffff0;
	      *ret_addr = memman_alloc((struct MEMMAN*)(ebx + ds_base), ecx);
	      break;

	case 10:
	/******************* free malloc *******************/
	/*		ebx: address of memman 		*/
	/*		eax: address which will be released	*/
	/*		ecx: the number of the bytes will be freed	*/
	/************************************************/
	      ecx = (ecx + 0x0f) & 0xfffffff0;
	      memman_free((struct MEMMAN*)(ebx+ds_base), eax, ecx);
	      break;

	case 11:
	/******************* free malloc *******************/
	/*		ebx: number of the window		*/
	/*		esi: x					*/
	/*		edi: y					*/
	/************************************************/
	      sht = (struct SHEET*)( ebx & 0xfffffffe);
	      sht->buf[sht->bxsize * edi + esi] = eax;
	      if(!( ebx & 1))
		sheet_refresh(sht, esi, edi, esi+1, edi+1);
	      break;

	case 12:
	/******************* refreshwin ********************/
	/*		ebx: number of the window		*/
	/*		eax: x0					*/
	/*		ecx: y0					*/
	/*		esi: x1					*/
	/*		edi: y1					*/
	/************************************************/
	      sht = (struct SHEET*) ebx;
	      sheet_refresh(sht, eax, ecx, esi, edi);
	      break;

	case 13:
	/****************** api_linewin ********************/
	/*		ebx: number of the window		*/
	/*		eax: x0					*/
	/*		ecx: y0					*/
	/*		esi: x1					*/
	/*		edi: y1					*/
	/*		ebp: color				*/
	/************************************************/
	      sht = (struct SHEET*)( ebx & 0xfffffffe);
	      os_api_linewin(sht, eax, ecx, esi, edi, ebp);
	      if(! (ebx & 1) )
		sheet_refresh(sht, eax, ecx, esi+1, edi+1);
	      break;

	case 14:
	/****************** api_closewin *******************/
	/*		ebx: number of the window		*/
	/************************************************/
	      sheet_free((struct SHEET*) ebx);
	      break;

	case 15:
	/****************** api_getkey  ********************/
	/*		eax: 0, return -1 if not get key		*/
	/*		       1, sleep until getting the key		*/
	/*		return:	received key's text code		*/
	/************************************************/
	      while(1)
	      {
		io_cli();
		if(!fifo32_status(&task->fifo))
		{
		      if(eax)
			task_sleep(task);
		      else
		      {
			io_sti();
			*ret_addr = -1;
			return 0;
		      }
		}
		data = fifo32_get(&task->fifo);
		io_sti();
		if(data <= 1)	// timer for cursor
		{
		      timer_init(cons->timer, &task->fifo, 1);
		      timer_settime(cons->timer, 50);
		}
		else if(data == CURSOR_ON)
		      cons->cur_c = COL8_FFFFFF;
		else if(data == CURSOR_OFF)
		      cons->cur_c = -1;
		else if(data == CONS_CLOSE)
		{
		      timer_cancel(cons->timer);
		      io_cli();
		      fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);
		      cons->sht = 0;
		      io_sti();
		}
		else if(data >= KEYBOARD_DATA0)
		{
		      *ret_addr = data - KEYBOARD_DATA0;
		      return 0;
		}
	      }
	      break;

	case 16:
	/******************* timer alloc ********************/
	/*		return:	timer number			*/
	/************************************************/
	      *ret_addr = (int) timer_alloc();
	      ((struct TIMER*)*ret_addr)->app_timer = 1;
	      break;

	case 17:
	/******************* timer init *********************/
	/*		ebx: timer number			*/
	/*		eax: data				*/
	/************************************************/
	      timer_init((struct TIMER*) ebx, &task->fifo, eax + KEYBOARD_DATA0);
	      break;

	case 18:
	/******************* timer set  *********************/
	/*		ebx: timer number			*/
	/*		eax: time				*/
	/************************************************/
	      timer_settime((struct TIMER*)ebx, eax);
	      break;

	case 19:
	/******************* timer free *********************/
	/*		ebx: timer number			*/
	/************************************************/
	      timer_free((struct TIMER*)ebx);
	      break;

	case 20:
	/********************* beep ***********************/
	/*		eax: mhz of the sound			*/
	/************************************************/
	      if(!eax)
	      {
		data = io_in8(0x61);
		io_out8(0x61, data & 0x0d);
	      }
	      else
	      {
		data = 1193180000 / eax;
		io_out8(0x43, 0xb6);
		io_out8(0x42, data & 0xff);
		io_out8(0x42, data >> 8);
		data = io_in8(0x61);
		io_out8(0x61, (data | 0x03) & 0x0f);
	      }
	      break;

	case 21:
	/******************* file open *********************/
	/*		ebx: file's name				*/
	/*		return:	filehandle			*/
	/*		          	0, if fail to open			*/
	/************************************************/
	      for(i = 0; i < 8; i++)
		if(!task->fhandle[i].buf)
		      break;
	      fh = &task->fhandle[i];
	      *ret_addr = 0;
	      if(i < 8)
	      {
		finfo = file_search((char*)ebx + ds_base, (struct FILE_INFO*)
		(DISKIMG_ADDR+DIR_TABLE_ADDR), DISK_MAX_FILES);
		if(finfo)
		{
		      *ret_addr = (int)fh;
		      fh->buf = (char*)memman_alloc_4k(memman, finfo->size);
		      fh->size = finfo->size;
		      fh->pos = 0;
		      fh->buf = file_loadfile2(finfo->clustno, &fh->size, task->fat);
		}	
	      }
	      break;

	case 22:
	/******************* file close *********************/
	/*		eax: filehandle				*/
	/************************************************/
	      fh = (struct FILEHANDLE*)eax;
	      memman_free_4k(memman, (int)fh->buf, fh->size);
	      fh->buf = 0;
	      break;

	case 23:
	/*******************  file seek *********************/
	/*		eax: filehandle				*/
	/*		ecx: seek mode				*/
	/*		       0, from the head			*/
	/*		       1, from the current position		*/
	/*		       2, from the end			*/
	/*		ecx: offset				*/
	/************************************************/
	      fh = (struct FILEHANDLE*)eax;
	      if(ecx == 0)
		fh->pos = ebx;
	      else if(ecx == 1)
		fh->pos += ebx;
	      else if(ecx == 2)
		fh->pos = fh->size + ebx;
	      if(fh->pos < 0)
		fh->pos = 0;
	      if(fh->pos > fh->size)
		fh->pos = fh->size;
	      break;

	case 24:
	/******************** file size *********************/
	/*		eax: filehandle				*/
	/*		ecx: mode				*/
	/*		       0, file size				*/
	/*		       1, the bytes between the head		*/ 
	/*		           and the current position.		*/
	/*		       2, the bytes between the end		*/ 
	/*		           and the current position.		*/
	/*		return: result of the mode			*/
	/************************************************/
	      fh = (struct FILEHANDLE*) eax;
	      if(ecx == 0)
		*ret_addr = fh->size;
	      else if(ecx == 1)
		*ret_addr = fh->pos;
	      else if(ecx == 2)
		*ret_addr = fh->pos - fh->size;
	      break;

	case 25:
	/*******************  file seek *********************/
	/*		eax: filehandle				*/
	/*		ebx: buffer				*/
	/*		ecx: max read bytes			*/
	/*		return: bytes read this time		*/
	/************************************************/
	      fh = (struct FILEHANDLE*) eax;
	      for(i = 0; i < ecx; i++)
	      {
		if(fh->pos == fh->size)
		      break;
		*((char*)ebx + ds_base + i) = fh->buf[fh->pos];
		fh->pos++;
	      }
	      *ret_addr = i;
	      break;

	case 26:
	/****************** get cmdline ********************/
	/*		ebx: buffer				*/
	/*		ecx: size of the buffer			*/
	/*		return: bytes in the buffer			*/
	/************************************************/
	      for(i = 0 ; i < ecx; i++)
	      {
		*((char*)ebx + ds_base + i) = task->cmdline[i];
		if(!task->cmdline[i])
		      break;
	      }
	      *ret_addr = i;
	      break;

	case 27:
	/*************** get langmode info ******************/
	/*		return: langmode				*/
	/************************************************/
	      *ret_addr = task->langmode;
	      break;
      }
      return 0;
}

int* inthandler0c(int* esp)
{
      struct TASK* task = task_now();
      struct CONSOLE* cons = task->cons;
      char str[30];
      cons_putstr(cons, "\nINT 0C :\nStack Exception.\n");
      sprintf(str, "EIP = %08X\n", esp[11]);
      cons_putstr(cons, str);
      return &(task->tss.esp0);
}

int* inthandler0d(int *esp)
{
      struct TASK* task = task_now();
      struct CONSOLE *cons = task->cons;
      char str[30];
      cons_putstr(cons, "\nINT 0D  :\nGeneral Protected Exception.\n");
      sprintf(str, "EIP = %08X\n", esp[11]);
      cons_putstr(cons, str);

      return &(task->tss.esp0);
}

void os_api_linewin(struct SHEET* sht, int x0, int y0, int x1, int y1, int col)
{
      int i, x, y, len, dx, dy;

      dx = x1 - x0;
      dy = y1 - y1;
      x = x0 << 10;
      y = y0 << 10;

      if(dx < 0)	dx = -dx;
      if(dy < 0)	dy = -dy;

      if(dx >= dy)
      {
	len = dx + 1;
	if(x0 > x1)
	      dx = -1024;
	else
	      dx = 1024;
	if(y0 <= y1)
	      dy = ((y1 - y0 + 1) << 10) / len;
	else
	      dy = ((y1 - y0 - 1) << 10) / len;
      }
      else
      {
	len = dy + 1;
	if(y0 > y1)
	      dy = -1024;
	else
	      dy = 1024;
	if(x0 <= x1)
	      dx = ((x1 - x0 + 1) << 10) / len;
	else
	      dx = ((x1 - x0 - 1) << 10) / len;
      }
      for(i=0; i< len; i++)
      {
	sht->buf[(y>>10) * sht->bxsize + (x >> 10)] = col;
	x += dx;
	y += dy;
      }
      return;
}
