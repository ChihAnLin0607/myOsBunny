#include "bootpack.h"

void file_readfat(int *fat, unsigned char *img)
{
      int i, j;
      for(i=0, j=0; i<2880; i+=2, j+=3)
      {
	fat[i] = (img[j] | img[j+1] << 8) & 0xfff;
	fat[i+1] = (img[j+1] >> 4 | img[j+2] << 4) & 0xfff;
      }
      return;
}

void file_loadfile(int clustno, int size, char* buf, int* fat, char* img)
{
      int i;
      while(size > 0)
      {
	for(i=0; i<512; i++)
	      buf[i] = img[clustno*512 + i];
	size -= 512;
	buf += 512;
	clustno = fat[clustno];
      }
      return;
}

char* file_loadfile2(int clustno, int* psize, int* fat)
{
      int size = *psize, size2;
      struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
      char* buf, *buf2;
      buf = (char*)memman_alloc_4k(memman, size);
      file_loadfile(clustno, size, buf, fat, (char*)(DISKIMG_ADDR+CLUSTNO0_OFFSET));
      if(size >= 17)
      {
	size2 = tek_getsize(buf);
	if(size2 > 0)
	{
	      buf2 = (char*)memman_alloc_4k(memman, size2);
	      tek_decomp(buf, buf2, size2);
	      memman_free_4k(memman, (int)buf, size);
	      buf = buf2;
	      *psize = size2;
	}
      }
      return buf;
}

struct FILE_INFO *file_search(char* name, struct FILE_INFO* file_info, int max)
{
      int i, j;
      char s[12];
      for(i=0; i<11; i++)
	s[i] = ' ';
      for(i = 0, j = 0; name[i]; i++, j++)
      {
	if(j >= 11)
	      return 0;
	if(name[i] == '.' && j <= 8)
	      j = 7;
	else
	{
	      s[j] = name[i];
	      if('a' <= s[j] && s[j] <= 'z')
		s[j] -= 0x20;
	}
      }
      for(i = 0; i < DISK_MAX_FILES; i++)
      {
	if(file_info[i].name[0] == 0x00)
	      break;
	else if(!(file_info[i].type & 0x18))
	{
	      for(j=0; j<11; j++)
	      {
		if(file_info[i].name[j] != s[j])
		      break;
	      }
	      if(j == 11)
		return file_info+i;
	}
      }
      return 0;
}
