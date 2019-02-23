#include"apilib.h"

void HariMain()
{
      int fh;
      char c, cmdline[32], *p;

      api_cmdline(cmdline, 32);
      for(p = cmdline; *p > ' '; p++);
      for(; *p == ' '; p++);	

      fh = api_fopen(p);
      if(fh)
	while(api_fread(&c, 1, fh))
	      api_putchar(c);
      else
	api_putstr("File not found.\n");
      api_end();
}
