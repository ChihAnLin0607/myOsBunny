#include "bootpack.h"

#define FLAGS_OVERRUN	0x1

void fifo32_init(struct FIFO32* fifo, int size, int* buf, struct TASK* task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->top = 0;
	fifo->button = 0;
	fifo->task = task;
	return;
}

int fifo32_put(struct FIFO32* fifo, int data)
{
      if(!fifo->free)
      {
	fifo->flags |= FLAGS_OVERRUN;
	return -1;
      }
      fifo->buf[fifo->top++] = data;
      if(fifo->top == fifo->size)
	fifo->top = 0;
      fifo->free--;
      if(fifo->task)
      {
	if(fifo->task->flags != TASK_FLAG_RUNNING)
	      task_run(fifo->task, -1, 0);
      }
      return 0;
}

int fifo32_get(struct FIFO32* fifo)
{
	int data;
	if(fifo->free == fifo->size)
		return -1;
	data = fifo->buf[fifo->button++];
	if(fifo->button == fifo->size)
		fifo->button = 0;
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32* fifo)
{
	return fifo->size - fifo->free;
}
