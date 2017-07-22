// 6502 sim driver
// copyright (c) 2017 Michael T. Mayers

#include "mos6502.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t memory[65536];

void busWrite( uint16_t addr, uint8_t data )
{
	printf( "write<%04x> : %02x\n", addr, data );
	memory[addr] = data;
}
uint8_t busRead( uint16_t addr )
{
	uint8_t data = memory[addr];
	printf( "read<%04x> : %02x\n", addr, data );
	return data;
}

int main( int argc, const char** argv )
{
	for( int i=0; i<65536; i++ )
		memory[i] = 0;

	if( argc == 2 )
	{
		auto filename = argv[1];

		printf( "trying to load binary<%s>\n", filename );

		FILE* fin = fopen(filename,"rb");
		if( fin )
		{
			fseek(fin,0,SEEK_END);
			size_t size = ftell(fin);
			printf( "binary size<%zu>\n", size );
			assert(size<=65536);
			assert(size>0);
			fseek(fin,0,SEEK_SET);
			fread(memory,size,1,fin);
			fclose(fin);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
	}

	// set reset vector to 0x200
	memory[0xfffc] = 0x00;
	memory[0xfffd] = 0x02;

	mos6502 cpu(busRead,busWrite);
	cpu.Reset();
	for( int i=0; i<64; i++ )
	{
		auto PC = cpu.pc;
		auto SP = cpu.sp;
		auto X = cpu.X;
		auto Y = cpu.Y;
		auto A = cpu.A;
		auto ST = cpu.status;
		printf( "PC<%04x> SP<%04x> A<%02x> X<%02x> Y<%02x> ST<%02x>\n", PC, SP, A, X, Y, ST );
		cpu.Run(1);
	}
	return 0;
}