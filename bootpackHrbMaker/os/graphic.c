#include "bootpack.h"

void putstring8_asc_sht(struct SHEET* sht, int x, int y, int color, int bgcolor, char* s, int len)
{
      struct TASK* task = task_now();
      boxfill8(sht->buf, sht->bxsize, bgcolor, x, y, x + len*8 -1, y+15);
      if(task->langmode != ENG && task->langbyte1)
      {
	putstring8_asc(sht->buf, sht->bxsize, x, y, color, s);
	sheet_refresh(sht, x-CHAR_WIDTH, y, x+len*CHAR_WIDTH, y+16);
      }
      else
      {
	putstring8_asc(sht->buf, sht->bxsize, x, y, color, s);
	sheet_refresh(sht, x, y, x + len*CHAR_WIDTH, y+16);
      }
      return;
}

void putblock8_8(char* vram, int vxsize, int xsize, int ysize, int x0, int y0, char* buf, int bxsize)
{
	int x, y;
	for(y = 0; y < ysize; y++)
	{
		for(x = 0; x < xsize; x++)
		{
			int start_point = x0 + y0*vxsize;
			vram[start_point + x + vxsize*y] = buf[x + y*bxsize];
		}
	}
	return;
}

void init_mouse_cursor8(char* mouse, char bgcolor)
{
	static char cursor[16][16] = 
	{
		"************..",
		"*00000000000*...",
		"*0000000000*....",
		"*000000000*.....",
		"*00000000*......",
		"*0000000*.......",
		"*0000000*.......",
		"*00000000*......",
		"*0000**000*.....",
		"*000*..*000*....",
		"*00*....*000*...",
		"*0*......*000*..",
		"**........*000*.",
		"*..........*000*",
		"............*00*",
		".............***",
	};
	int x, y;
	for(y=0; y<16; y++)
	{
		for(x=0; x<16; x++)
		{
			if(cursor[x][y] == '*')
				mouse[x + 16*y] = COL8_000000;
			else if(cursor[x][y] == '0')
				mouse[x + 16*y] = COL8_FFFFFF;
			else
				mouse[x + 16*y ] = bgcolor;
		}
	}
}

void putstring8_asc(char* vram, int scrnx, int x, int y, char color, unsigned char* str)
{
      extern char hankaku[4096];
      struct TASK* task = task_now();
      char* nihongo = (char*)NIHONGO_ADDR;
      char* font;
      int i = 0, k, t;

      if(task->langmode == ENG)
      {
 	for(i=0; str[i] != 0x00; i++)
      	{
	      putfont8(vram, scrnx, x, y, color,  hankaku + str[i] *16);
	      x += 8;
	}
      }
      else if(task->langmode == JAN)
      {
 	for(i=0; str[i] != 0x00; i++)
      	{
	      if(!task->langbyte1)
	     {
		if((0x81 <= str[i] && str[i] <= 0x9f) || (0xe0 <= str[i] && str[i] <= 0xfc))
		      task->langbyte1 = str[i];
		else
		      putfont8(vram, scrnx, x, y, color,  nihongo + str[i] *16);
	      }
	      else
	      {
		if(0x81 <= task->langbyte1 && task->langbyte1 <= 0x9f)
		      k = (task->langbyte1 - 0x81) * 2;
		else
		      k = (task->langbyte1 - 0xe0) * 2 + 62;
		
		if(0x40 <= str[i] && str[i] <= 0x7e)
		      t = str[i] - 0x40;
		else if(0x80 <= str[i] && str[i] <= 0x9e)
		      t = str[i] - 0x80 + 63;
		else
		{
		      t = str[i] - 0x9f;
		      k++;
		}
		task->langbyte1 = 0;
		font = nihongo + 256*16 + (k*94 + t) * 32;
		putfont8(vram, scrnx, x - CHAR_WIDTH, y, color, font);
		putfont8(vram, scrnx, x, y, color, font + 16);
	      }
	      x += CHAR_WIDTH;
	}
      }
      else if(task->langmode == ECU)
      {
	for(i=0; str[i] != 0x00; i++)
	{
	      if(!task->langbyte1)
	      {
		if(0x81 <= str[i] && str[i] <= 0xfe)
		      task->langbyte1 = str[i];
		else
		      putfont8(vram, scrnx, x, y, color, nihongo + str[i] * 16);
	      }
	      else
	      {
		k = task->langbyte1 - 0xa1;
		t = str[i] - 0xa1;
		task->langbyte1 = 0;
		font = nihongo + 256 * 16 + (k * 94 + t) * 32;
		putfont8(vram, scrnx, x-CHAR_WIDTH, y, color, font);
		putfont8(vram, scrnx, x, y, color, font+16);
	      }
	      x += CHAR_WIDTH;
	}
      }
      return;
}

void putfont8(char* vram, int scrnx, int x, int y, char color, char* font)
{
	int i;
	char row;
	for(i=0; i<16; i++)
	{
		char* vram_addr =  vram + x + ( y + i ) * scrnx;
		row = font[i];
		if(row & 0x80) vram_addr[0] = color;
		if(row & 0x40) vram_addr[1] = color;
		if(row & 0x20) vram_addr[2] = color;
		if(row & 0x10) vram_addr[3] = color;
		if(row & 0x08) vram_addr[4] = color;
		if(row & 0x04) vram_addr[5] = color;
		if(row & 0x02) vram_addr[6] = color;
		if(row & 0x01) vram_addr[7] = color;
	}
	return;
}

void init_scrn(short scrnx, short scrny, char* buf)
{	
      boxfill8(buf, scrnx, COL8_008484	, 0, 0, scrnx - 1, scrny - 29);
      boxfill8(buf, scrnx, COL8_C6C6C6	, 0, scrny- 28, scrnx - 1, scrny-28);
      boxfill8(buf, scrnx, COL8_FFFFFF	, 0, scrny- 27, scrnx - 1,scrny-27);
      boxfill8(buf, scrnx, COL8_C6C6C6	, 0, scrny- 26, scrnx - 1, scrny-1);

      boxfill8(buf, scrnx, COL8_FFFFFF	, 3, scrny-24, 59,scrny-24);
      boxfill8(buf, scrnx, COL8_FFFFFF	, 2, scrny- 24, 2, scrny- 4);
      boxfill8(buf, scrnx, COL8_848484	, 3, scrny- 4, 59, scrny- 4);
      boxfill8(buf, scrnx, COL8_848484	, 59, scrny- 23, 59,scrny-5);
      boxfill8(buf, scrnx, COL8_000000	, 2, scrny- 3, 59, scrny- 3);
      boxfill8(buf, scrnx, COL8_000000	, 60, scrny- 24, 60,scrny-3);
      boxfill8(buf, scrnx, COL8_848484	, scrnx-47, scrny-24, scrnx - 4, scrny- 24);
      boxfill8(buf, scrnx, COL8_848484	, scrnx-47, scrny-23, scrnx - 47, scrny- 4);
      boxfill8(buf, scrnx, COL8_FFFFFF	, scrnx-47, scrny-3, scrnx - 4, scrny- 3);
      boxfill8(buf, scrnx, COL8_FFFFFF	, scrnx-3, scrny-24, scrnx - 3, scrny- 3);
	

}

void boxfill8(unsigned char* buf, int xsize, unsigned char color, int x_start, int y_start, int x_end, int y_end)
{
	int i, j;
	for(i = x_start; i<=x_end; i++)
	{
		for(j=y_start; j<=y_end; j++)
			*(buf + j*xsize + i) = color;
	}
	return;
}

void init_palette()
{
      static  unsigned char table_rgb[16*3]=
      {
	0x00, 	0x00,	0x00,
	0xff, 	0x00,	0x00,
	0x00,	0xff,	0x00,
	0xff,	0xff, 	0x00,
	0x00,	0x00,	0xff,
	0xff, 	0x00, 	0xff,
	0x00, 	0xff,	0xff,
	0xff,	0xff,	0xff,
	0xc6,	0xc6,	0xc6,
	0x84,	0x00,	0x00,
	0x00,	0x84,	0x00,
	0x84,	0x84,	0x00,
	0x00,	0x00,	0x84,
	0x84,	0x00,	0x84,
	0x00,	0x84,	0x84,
	0x84,	0x84,	0x84
      };
      unsigned char table2[216*3];
      int r, g, b;
	
      set_palette(0, 15, table_rgb);
      for(b=0; b < 6; b++)
      {
	for(g=0; g<6; g++)
	{
	      for(r=0; r<6; r++)
	      {
		table2[(r + g*6 + b*36) * 3 + 0] = r * 51;
		table2[(r + g*6 + b*36) * 3 + 1] = g * 51;
		table2[(r + g*6 + b*36) * 3 + 2] = b * 51;
	      }
	}
      }
      set_palette(16, 231, table2);
      return;
}

void set_palette(int start, int end, unsigned char* rgb)
{
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8, start);
	for(i=start; i<=end; i++)
	{
		io_out8(0x03c9, rgb[0]/4);
		io_out8(0x03c9, rgb[1]/4);
		io_out8(0x03c9, rgb[2]/4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void make_window8(unsigned char* buf, int xsize, int ysize, char* title, char active)
{
      
      boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize-1, 0);
      boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize-2, 1);
      boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize-1);
      boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize-2);
      boxfill8(buf, xsize, COL8_848484, xsize-2, 1, xsize-2, ysize-2);
      boxfill8(buf, xsize, COL8_000000, xsize-1, 0, xsize-1, ysize-1);
      boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize-3, ysize-3);
      boxfill8(buf, xsize, COL8_848484, 1, ysize-2, xsize-2, ysize-2);
      boxfill8(buf, xsize, COL8_000000, 0, ysize-1, xsize-1, ysize-1);
      make_wtitle8(buf, xsize, title, active);
      return;
}

void make_wtitle8(unsigned char* buf, int xsize, char* title, char active)
{
      static char closeBtn[14][16] = {
      	"OOOOOOOOOOOOOOO@",
      	"OQQQQQQQQQQQQQ$@",
      	"OQQQQQQQQQQQQQ$@",
      	"OQQQ@@QQQQ@@QQ$@",
      	"OQQQQ@@QQ@@QQQ$@",
      	"OQQQQQ@@@@QQQQ$@",
      	"OQQQQQQ@@QQQQQ$@",
      	"OQQQQQ@@@@QQQQ$@",
      	"OQQQQ@@QQ@@QQQ$@",
      	"OQQQ@@QQQQ@@QQ$@",
      	"OQQQQQQQQQQQQQ$@",
      	"OQQQQQQQQQQQQQ$@",
      	"O$$$$$$$$$$$$$$@",
      	"@@@@@@@@@@@@@@@@"
      };

      int x, y;
      char  color, title_c, title_bar_c;
      if(active)
      {
	title_c = COL8_FFFFFF;
	title_bar_c = COL8_000084;
      }
      else
      {
	title_c = COL8_C6C6C6;
	title_bar_c = COL8_848484;
      }
      boxfill8(buf, xsize, title_bar_c, 3, 3, xsize-4, 20);
      putstring8_asc(buf, xsize, 24, 4, title_c, title);
      
      for(y = 0; y<14; y++)
      {
	for(x=0; x<16; x++)
	{
	      color = closeBtn[y][x];
	      if(color == '@')	color = COL8_000000;
	      else if(color == '$')	color = COL8_848484;
	      else if(color == 'Q')	color = COL8_C6C6C6;
	      else			color = COL8_FFFFFF;
	      buf[(5+y)*xsize + xsize - 21 + x] = color;
	}
      }
      return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int color)
{
      int x1 = x0 + sx;
      int y1 = y0 + sy;
      boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-2, y0-3, x1+1, y0-3);
      boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-3, y0-3, x0-3, y1+1);
      boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0-3, y1+2, x1+1, y1+2);
      boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1+2, y0-3, x1+2, y1+2);
      boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-1, y0-2, x1, y0-2);
      boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-2, y0-2, x0-2, y1);
      boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0-2, y1+1, x1, y1+1);
      boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1+1, y0-2, x1+1, y1+1);
      boxfill8(sht->buf, sht->bxsize, color, x0-1, y0-1, x1, y1);
      return;
}

void change_wtitle8(struct SHEET* sht, char act)
{
      int x, y, xsize = sht->bxsize;
      char c = 0, tc_new, tbc_new, tc_old, tbc_old, *buf = sht->buf;  //t: title, tb: title bar
      if(act)
      {
	tc_new = COL8_FFFFFF;
	tbc_new = COL8_000084;
	tc_old = COL8_C6C6C6;
	tbc_old = COL8_848484;
      }
      else
      {
	tc_new = COL8_C6C6C6;
	tbc_new = COL8_848484;
	tc_old = COL8_FFFFFF;
	tbc_old = COL8_000084;
      }
      for(y = 3; y <= 20; y++)
      {
	for(x = 3; x <= xsize - 4; x++)
	{
	      c = buf[y*xsize+x];
	      if(c == tc_old && x <= xsize - 22)
		c = tc_new;
	      else if(c == tbc_old)
		c = tbc_new;
	      buf[y*xsize + x] = c;
	}
      }
      sheet_refresh(sht, 3, 3, xsize, 21);
      return;
}
