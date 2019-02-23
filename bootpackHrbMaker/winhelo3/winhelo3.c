#include"apilib.h"

void HariMain()
{
      char* buf;
      int win;

      api_initmalloc();
      buf = api_malloc(150*50);
      win = api_openwin(buf, 150, 50, -1, "hello");
      api_boxfillwin(win, 8, 36, 141, 43, 3);
      api_putstrwin(win, 28, 28, 0, 12, "Hello, World");
      while(1)
	if(api_getkey(1) == 0x0a)
	      break;
      api_end();
}
