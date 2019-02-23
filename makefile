FILES = ipl.asm makefile hrb/hello3.hrb hrb/hello4.hrb hrb/hello5.hrb hrb/a.hrb hrb/winhello.hrb hrb/winhelo2.hrb \
		hrb/winhelo3.hrb hrb/star1.hrb hrb/star2.hrb hrb/lines.hrb hrb/walk.hrb hrb/noodle.hrb hrb/color.hrb hrb/color2.hrb \
		hrb/sosu.hrb hrb/cat.hrb hrb/iroha.hrb hrb/chklang.hrb nihongo.fnt ipl10.nas

USB_ADDR = /media/linchihan/1C05-B17A

default:
	make install

%.bin: %.asm
	nasm -f bin $*.asm -o $*.bin -l lst/$*.lst

hrb/%.hrb:
	make -C hrb $*.hrb

bootpack.hrb:
	cp $(USB_ADDR)/bootpack.hrb .

asmfunc.o: asmfunc.asm
	nasm -f coff asmfunc.asm
	rm -f $(USB_ADDR)/asmfunc.o
	cp asmfunc.o $(USB_ADDR)/

alloca.o: alloca.asm
	nasm -f coff alloca.asm
	cp alloca.o $(USB_ADDR)/

haribote.sys: asmhead.bin bootpack.hrb
	cat asmhead.bin bootpack.hrb > haribote.sys

haribote.img: ipl.bin haribote.sys $(FILES)
	rm -f haribote.img
	dd if=/dev/zero of=haribote.img bs=1474560 count=1
	sudo mkfs.vfat -F 12 haribote.img
	dd conv=notrunc if=ipl.bin of=haribote.img
	sudo mount haribote.img mountdir
	sudo cp haribote.sys mountdir
	sudo cp $(FILES) mountdir
	sudo umount mountdir

install: haribote.img
	rm -f $(USB_ADDR)/helloos.img
	cp haribote.img $(USB_ADDR)/helloos.img
	rm *.sys

clean:
	rm hrb/*.hrb
	rm bootpack.hrb
