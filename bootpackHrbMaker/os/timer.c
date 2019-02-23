#include "bootpack.h"

struct TIMERCTL timerctl;
extern struct TIMER* task_timer;

void init_pit()
{
      io_out8(PIT_CTRL, 0x34);
      io_out8(PIT_CNT0, 0x9c);
      io_out8(PIT_CNT0, 0x2e);
      timerctl.counter = 0;
      timerctl.next = 0xffffffff;
      int i;
      for(i=0; i<MAX_TIMER; i++)
	timerctl.timers0[i].flags = 0;

      struct TIMER* timer = timer_alloc();
      timer->timeout = 0xffffffff;
      timer->flags = TIMER_FLAGS_USING;
      timer->next = 0;
      timerctl.timers_head = timer;
      timerctl.next = 0xffffffff;
      return;
}

struct TIMER* timer_alloc()
{
      int i;
      for(i=0; i<MAX_TIMER; i++)
      {
	if(!timerctl.timers0[i].flags)
	{
	      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
	      timerctl.timers0[i].app_timer = 0;
	      return &timerctl.timers0[i];
	}
      }
      return 0;
}

void timer_free(struct TIMER* timer)
{
      timer->flags = 0;      
      return;
}

void timer_init(struct TIMER* timer, struct FIFO32* fifo, int data)
{
      timer->fifo = fifo;
      timer->data = data;
      return;
}

void timer_settime(struct TIMER* timer, unsigned int timeout)
{
      int eflags = 0;
      timer->timeout = timeout + timerctl.counter;
      timer->flags = TIMER_FLAGS_USING;

      eflags = io_load_eflags();
      io_cli();

      if(timer->timeout <= timerctl.timers_head->timeout)
      {
	timer->next = timerctl.timers_head;
	timerctl.timers_head = timer;
 	timerctl.next = timer->timeout;
	io_store_eflags(eflags);
	return; 
      }

      struct TIMER* timer_ptr = timerctl.timers_head;
      
      for(; timer_ptr->next; timer_ptr = timer_ptr->next)
      {
	if(timer_ptr->next->timeout >= timer->timeout)
	{
	      timer->next = timer_ptr->next;
	      timer_ptr->next = timer;
	      io_store_eflags(eflags);
	      return;
	}
      }
      return;
}

void inthandler20(int* esp)
{
      char is_task_timer_timeout = 0;
      io_out8(PIC_M_OCW2, 0x60);
      timerctl.counter++;
      
      if(timerctl.next > timerctl.counter)
	return;
          
      struct TIMER* timer = timerctl.timers_head;
      for(; timer; timer=timer->next)
      {
	if(timer->timeout > timerctl.counter)
	      break;
	timer->flags = TIMER_FLAGS_ALLOC;
	if(timer != task_timer)
	      fifo32_put(timer->fifo, timer->data);
	else
	      is_task_timer_timeout = 1;
      }
      timerctl.timers_head = timer;
      timerctl.next = timerctl.timers_head->timeout;
      if(is_task_timer_timeout)
	task_switch();
      return;
}

int timer_cancel(struct TIMER* timer)
{
      int e;
      struct TIMER* t;
      e = io_load_eflags();
      io_cli();
      if(timer->flags == TIMER_FLAGS_USING)
      {
	if(timer == timerctl.timers_head)
	{
	      t = timer->next;
	      timerctl.timers_head = t;
	      timerctl.next = t->timeout;
	}
	else
	{
	      for(t = timerctl.timers_head; t->next != timer; t = t->next);
	      t->next = timer->next;
	}
	timer->flags = TIMER_FLAGS_ALLOC;
	io_store_eflags(e);
	return 1;
      }
      io_store_eflags(e);
      return 0;
}

void timer_cancelall(struct FIFO32* fifo)
{
      int e, i;
      struct TIMER* t;
      e = io_load_eflags();
      io_cli();
      for(i=0; i<MAX_TIMER; i++)
      {
	t = &timerctl.timers0[i];
	if(t->flags && t->app_timer && t->fifo == fifo)
	{
	      timer_cancel(t);
	      timer_free(t);
	}
      }
      io_store_eflags(e);
      return;
}
