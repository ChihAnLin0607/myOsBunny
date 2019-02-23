/*01*/	void api_putchar(int c);
/*02*/	void api_putstr(char *s);
/*03*/	void api_putstr_len(char *s, int l);
/*04*/	void api_end(void);
/*05*/	int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
/*06*/	void api_putstrwin(int win, int x, int y, int col, int len, char *str);
/*07*/	void api_boxfillwin(int win, int x0, int y0, int x1, int y1, int col);
/*08*/	void api_initmalloc(void);
/*09*/	char *api_malloc(int size);
/*10*/	void api_free(char *addr, int size);
/*11*/	void api_point(int win, int x, int y, int col);
/*12*/	void api_refreshwin(int win, int x0, int y0, int x1, int y1);
/*13*/	void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
/*14*/	void api_closewin(int win);
/*15*/	int api_getkey(int mode);
/*16*/	int api_alloctimer(void);
/*17*/	void api_inittimer(int timer, int data);
/*18*/	void api_settimer(int timer, int time);
/*19*/	void api_freetimer(int timer);
/*20*/	void api_beep(int tone);
/*21*/	int api_fopen(char* fname);
/*22*/	void api_fclose(int fhandle);
/*23*/	void api_fseek(int fhandle, int offset, int mode);
/*24*/	int api_fsize(int fhandle, int mode);
/*25*/	int api_fread(char* buf, int maxsize, int fhandle);
/*26*/ 	int api_cmdline(char* buf, int maxsize);
/*27*/	int api_getlang();

