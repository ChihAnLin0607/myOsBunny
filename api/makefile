API_FILES = api001.o api002.o api003.o api004.o api005.o api006.o api007.o api008.o api009.o api010.o \
			api011.o api012.o api013.o api014.o api015.o api016.o api017.o api018.o api019.o api020.o \
			api021.o api022.o api023.o api024.o api025.o api026.o api027.o

default:
	make api

api: $(API_FILES)
	cp $(API_FILES) /media/linchihan/CENA_X64FRE/

%.o: %.asm
	nasm -f coff $*.asm

clean:
	rm *.o
