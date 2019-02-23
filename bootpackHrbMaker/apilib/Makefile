OBJS_API =	api001.o api002.o api003.o api004.o api005.o api006.o api007.o api008.o \
		api009.o api010.o api011.o api012.o api013.o api014.o api015.o api016.o \
		api017.o api018.o api019.o api020.o api021.o api022.o api023.o api024.o \
		api025.o api026.o api027.o alloca.o

TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = ../haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GOLIB = $(TOOLPATH)golib00.exe
COPY     = copy
DEL      = del


default :
	$(MAKE) apilib.lib

apilib.lib: Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib


src_only :
	-$(DEL) apilib.lib
