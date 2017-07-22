all:
	cc65 -t none test1.c -o gen/test1.s
	ca65 -t none gen/test1.s -o gen/test1.o -l gen/test1.lst
	ca65 -t none crt0.s -o gen/crt0.o -l gen/crt0.lst
	ld65 -v -vm -C test.cfg -m gen/test1.map -o gen/test1.bin gen/crt0.o test1.o supervision.lib