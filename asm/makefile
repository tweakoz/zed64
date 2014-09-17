CFLAGS= --add-source
SYS=c64

all:
	64tass -C -a -B -i ./inl.s -L ./gen/inl.lst -f -b -o ./gen/main.prg
	#cc65 yo.c -t none -o yo.s
	#ca65 yo.s -o yo.o
	#ca65 crt0.s -o crt0.o
	#ld65 yo.o crt0.o -t none -o yo.exe
	#cat ./gen/inl.lst
	od -w65536 -A n -t x1 ./gen/main.prg > ./rawcode.txt
	dd if=./rawcode.txt of=../rtl/code.txt bs=1 skip=1 > yo.txt



exec:	all
	x64 -ntsc -VICIIdsize -VICIIhwscale ./gen/main.prg 

clean:
	rm gen/*
