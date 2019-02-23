TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

OBJS_BOOTPACK =	bootpack.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj sheet.obj memory.obj timer.obj mtask.obj file.obj console.obj tek.obj

MAKE		= $(TOOLPATH)make.exe -r
NASK		= $(TOOLPATH)nask.exe
CC1		= $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK	= $(TOOLPATH)gas2nask.exe -a
OBJ2BIM	= $(TOOLPATH)obj2bim.exe
MAKEFONT	= $(TOOLPATH)makefont.exe
BIN2OBJ 	= $(TOOLPATH)bin2obj.exe
BIM2HRB	= $(TOOLPATH)bim2hrb.exe
RULEFILE 	= ../haribote.rul
EDIMG		= $(TOOLPATH)edimg.exe
IMGTOL	= $(TOOLPATH)imgtol.com
GOLIB		= $(TOOLPATH)golib00.exe
COPY		= copy
DEL		= del


default :
	$(MAKE) bootpack.hrb

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK) asmfunc.o
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	$(COPY) bootpack.hrb ..
#	$(MAKE) clean

%.gas : %.c bootpack.h Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj	
	-$(DEL) *.bin
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.bim

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
