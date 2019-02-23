#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end)
{
      char flag486up = 0;
      unsigned int eflag, cr0, mem_size;
      eflag = io_load_eflags();
      eflag |= EFLAGS_AC_BIT;
      io_store_eflags(eflag);
      eflag = io_load_eflags();
      if(eflag & EFLAGS_AC_BIT)
	flag486up = 1;

      eflag &= ~EFLAGS_AC_BIT;
      io_store_eflags(eflag);

      if(flag486up)
      {
	cr0 = load_cr0();
	cr0 |= CR0_CACHE_DISABLE;
	store_cr0(cr0);
      }

      mem_size = memtest_sub(start, end);

      if(flag486up)
      {
	cr0 = load_cr0();
	cr0 &= ~CR0_CACHE_DISABLE;
	store_cr0(cr0);
      }

      return mem_size;
}

void memman_init(struct MEMMAN *manager)
{
      manager->frees = 0;
      manager->maxfrees = 0;
      manager->lostsize = 0;
      manager->losts = 0;
      return;
}

unsigned int memman_total(struct MEMMAN* manager)
{
	unsigned  int i, total_free_size = 0;
	for(i=0; i<manager->frees; i++)
	      total_free_size += manager->free[i].size;
	return total_free_size;
}

unsigned int memman_alloc(struct MEMMAN* manager, unsigned int size)
{
      unsigned int i, addr;
      for(i=0; i<manager->frees; i++)
      {
	if(manager->free[i].size >= size)
	{
	      addr = manager->free[i].addr;
	      manager->free[i].addr += size;
	      manager->free[i].size -= size;
	      if(manager->free[i].size <= 0)
	      {
		manager->frees--;
		for(; i<manager->frees; i++)
		      manager->free[i] = manager->free[i+1];
	      }
	      return addr;
	}
      }
      return 0;
}

int memman_free(struct MEMMAN* manager, unsigned int addr, unsigned int size)
{
      int i = 0;
      for(i=0; i<manager->frees; i++)
      {
	if(manager->free[i].addr > addr);
	      break;
      }

      if(i > 0)
      {
	if(manager->free[i-1].addr + manager->free[i-1].size == addr)
	{
	      manager->free[i-1].size += size;
	      if(i<manager->frees && addr+size == manager->free[i].addr)
	     {
		manager->free[i-1].size += size;
		manager->frees--;
		for(; i<manager->frees; i++)
			manager->free[i] = manager->free[i+1];
	      }
	      return 0;
	}
      }

      if(i < manager->frees && addr+size == manager->free[i].addr)
      {
	manager->free[i].addr -= size;
      	manager->free[i].size += size;
      	return 0;
      }

      if(manager->frees < MEMMAN_FREES)
      {
      	int j;
      	for(j = manager->frees; j > i; j--)
	      manager->free[j] = manager->free[j-1];
            manager->frees++;
            if(manager->maxfrees < manager->frees)
	      manager->maxfrees = manager->frees;
            manager->free[i].size = size;
            manager->free[i].addr = addr;
            return 0;
      }
      manager->losts++;
      manager->lostsize += size;
      return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN* manager, unsigned int size)
{
      unsigned int addr;
      size = (size + 0xfff) & 0xfffff000;
      addr = memman_alloc(manager, size);
      return addr;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
       int ret;
       size = (size + 0xfff) & 0xfffff000;
       ret = memman_free(man, addr, size);
       return ret;
}
