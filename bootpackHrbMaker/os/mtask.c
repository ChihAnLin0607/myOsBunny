#include "bootpack.h"

struct TASKCTL* taskctl;
struct TIMER* task_timer;

struct TASK *task_init(struct MEMMAN *memman)
{
      struct TASK* task, *idle;
      struct SEGMENT_DESC* gdt = (struct SEGMENT*) GDT_ADDR;
      taskctl = (struct TASKCTL*)memman_alloc_4k(memman, sizeof(struct TASKCTL));
      int i;
      for(i=0; i<MAX_TASKS; i++)
      {
	taskctl->tasks0[i].flags = 0;
	taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
	taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
	set_segmdesc(gdt + TASK_GDT0 + i, sizeof(struct TSS32)-1, (int)&taskctl->tasks0[i].tss, 	AR_TSS32);
	set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, sizeof(taskctl->tasks0[i].ldt) - 1, 
	(int)taskctl->tasks0[i].ldt, AR_LDT);
      }
      for(i=0; i<MAX_TASKLEVELS; i++)
      {
	taskctl->level[i].top = 0;
	taskctl->level[i].now = 0;
      }

      task = task_alloc();
      task->priority = 2;
      task->level = 0;
      task_add(task);
      task_switchsub();
      load_tr(task->sel);

      idle = task_alloc();
      idle->tss.esp = memman_alloc_4k(memman, 64*1024) + 64*1024;
      idle->tss.eip = (int) &task_idle;
      idle->tss.es = 1*8;
      idle->tss.cs = 2*8;
      idle->tss.ss = 1*8;
      idle->tss.ds = 1*8;
      idle->tss.fs = 1*8;
      idle->tss.gs = 1*8;
      task_run(idle, MAX_TASKLEVELS-1, 1);

      task_timer = timer_alloc();
      timer_settime(task_timer, task->priority);
      return task;
}

struct TASK* task_now()
{
      struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
      return tl->tasks[tl->now];
}

void task_add(struct TASK* task)
{
      struct TASKLEVEL* tl = &taskctl->level[task->level];
      tl->tasks[tl->top++] = task;
      task->flags = TASK_FLAG_RUNNING;
      return;
}

void task_remove(struct TASK* task)
{
      int i;
      struct TASKLEVEL* tl = &taskctl->level[task->level];
	  
      for(i=0; i<tl->top; i++)
      {
	if(tl->tasks[i] == task)
	      break;
      }
      tl->top--;
      if(i < tl->now)
 	tl->now--;
      if(tl->now >= tl->top)
	tl->now = 0;
      task->flags = TASK_FLAG_SLEEPING;
      for(; i<tl->top; i++)
	tl->tasks[i] = tl->tasks[i+1];
      return;
}

void task_switchsub()
{
      int i;
      for(i=0; i<MAX_TASKLEVELS; i++)
      {
	if(taskctl->level[i].top > 0)
	      break;
      }
      taskctl->now_lv = i;
      taskctl->lv_change = 0;
      return;
}

struct TASK* task_alloc()
{
      int i;
      struct TASK* task;
      for(i=0;i <MAX_TASKS; i++)
      {
	if( !taskctl->tasks0[i].flags )
	{
	      task = &taskctl->tasks0[i];
	      task->flags = TASK_FLAG_SLEEPING;
	      task->tss.eflags = 0x202;
	      task->tss.eax = 0;
	      task->tss.ecx = 0;
	      task->tss.edx = 0;
	      task->tss.ebx = 0;
	      task->tss.ebp = 0;
	      task->tss.esi = 0;
	      task->tss.edi = 0;
	      task->tss.es = 0;
	      task->tss.ds = 0;
	      task->tss.fs = 0;
	      task->tss.gs = 0;
	      task->tss.iomap = 0x40000000;
	      task->tss.ss0 = 0;
	      return task;
	}
      }
      return 0;
}

void task_run(struct TASK* task, int level, int priority)
{
      if(level < 0)
	level = task->level;
      if(priority > 0)
	task->priority = priority;
      if(task->flags == TASK_FLAG_RUNNING && task->level != level)
	task_remove(task);
      if(task->flags != TASK_FLAG_RUNNING)
      {
	task->level = level;
	task_add(task);
      }
      taskctl->lv_change = 1;
      return;
}

void task_sleep(struct TASK *task)
{
      struct TASK *now_task;
      if(task->flags == TASK_FLAG_RUNNING)
      {
	now_task = task_now();
	task_remove(task);
	if(task == now_task)
	{
	      task_switchsub();
	      now_task = task_now();
	      farjmp(0, now_task->sel);
	}
      }
      return;
}

void task_switch()
{
      struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
      struct TASK* new_task, *now_task = tl->tasks[tl->now];
      tl->now++;
      if(tl->now == tl->top)
	tl->now = 0;
      if(taskctl->lv_change)
      {
	task_switchsub();
	tl = &taskctl->level[taskctl->now_lv];
      }

      new_task = tl->tasks[tl->now];
      timer_settime(task_timer, new_task->priority);
      if(new_task != now_task)
	farjmp(0, new_task->sel);
      
      return;
}

void task_idle()
{
      while(1)
	io_hlt();
}
