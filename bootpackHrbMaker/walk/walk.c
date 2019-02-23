#include"apilib.h"

void HariMain()
{
      char* buf;
      int win, data, x, y;
      api_initmalloc();
      buf = api_malloc(160*100);
      win = api_openwin(buf, 160, 100, -1, "walk");
      api_boxfillwin(win, 4, 24, 155, 95, 0);
      x = 76;
      y = 56;
      api_putstrwin(win, x, y, 3, 1, "*");
      while(1)
      {
	data = api_getkey(1);
	api_putstrwin(win, x, y, 0, 1, "*");
	if(data == '4' && x > 4)		x -= 8;
	if(data == '6' && x < 128)	x += 8;
	if(data == '8' && y > 24)		y -= 8;
	if(data == '2' && y < 80)		y += 8;
	if(data == 0x0a)			break;

	api_putstrwin(win, x, y, 3, 1, "*");	
      }
      api_closewin(win);
      api_end();
}
