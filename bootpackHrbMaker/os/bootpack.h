/*********************************   memory.c   *********************************/

#define EFLAGS_AC_BIT		0x40000
#define CR0_CACHE_DISABLE		0x60000000
#define MEMMAN_FREES		4094
#define MEMMAN_ADDR		0x3c0000

struct FREEINFO
{
      unsigned int addr, size;
};

struct MEMMAN
{
      int frees, maxfrees, lostsize, losts;
      struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *manager);
unsigned int memman_total(struct MEMMAN* manager);
unsigned int memman_alloc(struct MEMMAN* manager, unsigned int size);
int memman_free(struct MEMMAN* manager, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/*********************************   sheet.c   *********************************/

#define MAX_SHEETS			256

//flags
#define SHEET_USED			1
#define FLAG_WITH_CURSOR		0x20
#define FLAG_APP_WIN		0x10

struct SHEET
{
      struct SHTCTL* ctl;
      unsigned char *buf;
      int bxsize, bysize, vx0, vy0, col_inv, height, flags;
      struct TASK* task;
};

struct SHTCTL
{
      unsigned char *vram, *map;
      int xsize, ysize, top;
      struct SHEET *sheets[MAX_SHEETS];
      struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN* manager, unsigned char* vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char* buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET* sht, int height);
void sheet_refresh(struct SHEET *sht, int buf_x0, int buf_y0, int buf_x1, int buf_y1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshsub(struct SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refreshmap(struct SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0);

/*********************************   bootpack.c   *********************************/
#define ASCII_BACKSPACE		0x08
#define ASCII_RETURN			0x0a
#define KEYCMD_LED			0xed
#define KEY_TO_TASK_A		0
#define KEY_TO_CONSOLE		1
#define CURSOR_ON			2
#define CURSOR_OFF			3
#define CONS_CLOSE			4
#define CONS_X0			8
#define CONS_Y0			28
#define CONS_W			720			
#define CONS_H			384
#define CHAR_WIDTH			8
#define BOOTINFO_ADDR		0x0ff0
#define NIHONGO_ADDR		*((int*)0x0fe8)
#define CONS_SHT_DATA0		768
#define CONS_NSHT_DATA0		1024
#define ENG				0
#define JAN				1
#define ECU				2

struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char* vram;
};

struct FILEHANDLE
{
      char* buf;
      int size;
      int pos;
};

struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal);
struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal);

/********************************   asmkfunc.asm   ********************************/
 
void io_hlt();
void io_cli();
void io_sti();
void io_stihlt();
void io_out8(int port, int data);
int io_in8(int port);
int io_load_eflags();
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler0d(void);
void asm_inthandler0c(void);
int load_cr0();
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_os_api(void);
void start_app(int eip, int cs, int esp, int ds, int* tss_esp0);
void asm_end_app();

/*********************************   dsctbl.c   *********************************/
 
#define GDT_ADDR		0x00270000
#define IDT_ADDR		0x0026f800
#define LIMIT_GDT		0xffff
#define LIMIT_IDT		0x7ff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_LDT		0x0082

struct SEGMENT_DESC
{
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESC
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt();
void set_segmdesc(struct SEGMENT_DESC* sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESC* gd, int offset, int selector, int ar);

/*********************************   int.c   *********************************/

#define PIC_M_ICW1			0x0020
#define PIC_M_IMR			0x0021
#define PIC_M_ICW2			0x0021
#define PIC_M_ICW3			0x0021
#define PIC_M_ICW4			0x0021
#define PIC_S_ICW1			0x00a0
#define PIC_S_IMR			0x00a1
#define PIC_S_ICW2			0x00a1
#define PIC_S_ICW3			0x00a1
#define PIC_S_ICW4			0x00a1

#define PIC_M_OCW2			0x0020
#define PIC_S_OCW2			0x00a0

#define PORT_KEYDAT		0x0060
#define PORT_KEYSTA			0x64
#define PORT_KEYCMD		0x64
#define KEYSTA_SEND_NOT_READY	0x02
#define KEYCMD_WRITE_MODE	0x60
#define KBC_MODE			0x47
#define KEYCMD_SEND_TO_MOUSE	0xd4
#define MOUSE_CMD_ENABLE	0xf4

void init_pic();
void inthandler27(int *esp);

/*********************************   fifo.c   *********************************/

struct FIFO32
{
	int *buf;
	int top, button, size, free, flags;
	struct TASK* task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK* task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/*********************************   mouse.c   *********************************/

#define MOUSE_DATA0		512

struct MOUSE_DECODE
{
	unsigned char buf[3], phase;
	int x, y, low3;
};

void enable_mouse(struct FIFO32* fifo, int data0, struct MOUSE_DECODE* mdec);
int mouse_decode(struct MOUSE_DECODE* mdec, unsigned char dat);
void inthandler2c(int *esp);

/*********************************   keyboard.c   *********************************/

#define KEYBOARD_DATA0		256

void wait_KBC_sendready();
void init_keyboard();
void inthandler21(int *esp);

/*********************************   time.c   *********************************/

#define TIMER_FLAGS_ALLOC	1
#define TIMER_FLAGS_USING	2
#define	PIT_CTRL			0x43
#define	PIT_CNT0			0x40
#define MAX_TIMER			500

struct TIMER
{
      struct TIMER* next;
      unsigned int timeout;
      char flags, app_timer;
      struct FIFO32* fifo;
      int data;
};

struct TIMERCTL
{
      unsigned int counter, next;
      struct TIMER* timers_head;
      struct TIMER timers0[MAX_TIMER];
};

void init_pit();
void inthandler20(int* esp);
struct TIMER* timer_alloc();
void timer_free(struct TIMER* timer);
void timer_init(struct TIMER* timer, struct FIFO32* fifo, int data);
void timer_settime(struct TIMER* timer, unsigned int timeout);
int timer_cancel(struct TIMER* timer);
void timer_cancelall(struct FIFO32* fifo);

/*********************************   graphic.c   *********************************/

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF  7
#define COL8_C6C6C6	8
#define COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15
 
void init_palette();
void set_palette(int start, int end, unsigned char* rgb);
void putstring8_asc_sht(struct SHEET* sht, int x, int y, int color, int bgcolor, char* s, int len);
void boxfill8(unsigned char* buf, int xsize, unsigned char color, int x_start, int y_start, int x_end, int y_end);
void init_scrn(short scrnx, short scrny, char* buf);
void putfont8(char* vram, int scrnx, int x, int y, char color, char* font);
void putstring8_asc(char* vram, int scrnx, int x, int y, char color, unsigned char* str);
void init_mouse_cursor8(char* mouse, char bgcolor);
void putblock8_8(char* vram, int vxsize, int xsize, int ysize, int x0, int y0, char* buf, int bxsize);
void make_window8(unsigned char* buf, int xsize, int ysize, char* title, char active);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int color);
void make_wtitle8(unsigned char* buf, int xsize, char* title, char active);
void change_wtitle8(struct SHEET* sht, char act);

/*********************************   mtask.c   *********************************/

#define AR_TSS32			0x0089
#define MAX_TASKS			1000
#define MAX_TASKS_PER_LEVEL	100
#define MAX_TASKLEVELS		10
#define TASK_GDT0			3
#define TASK_FLAG_RUNNING	2
#define TASK_FLAG_SLEEPING	1
#define TASK_LV			-1

struct TSS32
{
      int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
      int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
      int es, cs, ss, ds, fs, gs;
      int ldtr, iomap;
};

struct TASK
{
      int sel, flags;
      int level, priority;
      struct FIFO32 fifo;
      struct TSS32 tss;
      struct SEGMENT_DESC ldt[2];
      struct CONSOLE* cons;
      int ds_base, cons_stack;
      struct FILEHANDLE* fhandle;
      int *fat;
      char* cmdline;
      unsigned char langmode, langbyte1;
};

struct TASKLEVEL
{
      int top;
      int now;
      struct TASK* tasks[MAX_TASKS_PER_LEVEL];
};

struct TASKCTL
{
      int now_lv;
      char lv_change;
      struct TASKLEVEL level[MAX_TASKLEVELS];
      struct TASK tasks0[MAX_TASKS];
};

struct TASK *task_init(struct MEMMAN *memman);
struct TASK* task_alloc();
void task_run(struct TASK* task, int level, int priority);
void task_switch();
void task_sleep(struct TASK *task);
struct TASK* task_now();
void task_add(struct TASK* task);
void task_remove(struct TASK* task);
void task_switchsub();
void task_idle();

/*********************************   console.c   *********************************/

#define FIFO_ADDR		*((int*)0x0fec)
#define SHTCTL_ADDR		*((int*)0x0fe4)
#define APP_ADDR		0x0fe8
#define APP_ATTR		0x60
#define LDT_FLAG		0x04

struct CONSOLE
{
      struct SHEET* sht;
      int cur_x, cur_y, cur_c;
      struct TIMER *timer;
};

void console_task(struct SHEET* sheet, unsigned int memtotal);
void cons_putchar(struct CONSOLE* cons, int chr, char move);
void cons_newline(struct CONSOLE* cons);
void cons_runcmd(char* cmdline, struct CONSOLE* cons, int *fat, unsigned int memtotal);
void cmd_langmode(struct CONSOLE* cons, char* cmdline);
void cmd_mem(struct CONSOLE* cons, unsigned int memtotal);
void cmd_clear(struct CONSOLE* cons);
void cmd_dir(struct CONSOLE* cons);
void cmd_start(struct CONSOLE* cons, char* cmdline, int memtotal);
void cmd_ncst(struct CONSOLE* cons, char* cmdline, int memtotal);
void cmd_exit(struct CONSOLE* cons, int *fat);
int cmd_app(struct CONSOLE* cons, int *fat, char* cmdline);
void cons_putstr(struct CONSOLE* cons, char* str);
void cons_putstr_len(struct CONSOLE* cons, char* str, int len);
int* os_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
void os_api_linewin(struct SHEET* sht, int x0, int y0, int x1, int y1, int col);
int* inthandler0d(int *esp);
int* inthandler0c(int* esp);

/**********************************   file.c   **********************************/

#define DISKIMG_ADDR		0x100000 
#define DISK_MAX_FILES		224
#define DIR_TABLE_ADDR		0x2600
#define FAT_OFFSET			0x200
#define CLUSTNO0_OFFSET		0x3e00

struct FILE_INFO
{
//      char reserve0[32];
      unsigned char name[8], ext[3], type;
      char reserve1[10];
      unsigned short time, date, clustno;
      unsigned int size;
};

void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char* buf, int* fat, char* img);
char* file_loadfile2(int clustno, int* psize, int* fat);
struct FILE_INFO *file_search(char* name, struct FILE_INFO* file_info, int max);

/**********************************   tek.c   **********************************/

int tek_getsize(unsigned char *p);
int tek_decomp(unsigned char *p, char *q, int size);


