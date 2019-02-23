TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GOLIB = $(TOOLPATH)golib00.exe
COPY     = copy
DEL      = del

default:
	$(MAKE) os
	$(MAKE) apps

os:
	$(MAKE) -C os

apps:
	$(MAKE) -C  apilib
	$(MAKE) -C  a
	$(MAKE) -C  hello3
	$(MAKE) -C  hello4
	$(MAKE) -C  hello5
	$(MAKE) -C  winhello
	$(MAKE) -C  winhelo2
	$(MAKE) -C  winhelo3
	$(MAKE) -C  star1
	$(MAKE) -C  star2
	$(MAKE) -C  lines
	$(MAKE) -C  walk
	$(MAKE) -C  noodle
	$(MAKE) -C  color
	$(MAKE) -C  color2
	$(MAKE) -C  sosu
	$(MAKE) -C  cat
	$(MAKE) -C  chklang

clean:
	$(DEL) app\*.hrb
	$(DEL) bootpack.hrb
	