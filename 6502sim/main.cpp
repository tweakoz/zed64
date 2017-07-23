// 6502 sim driver
// copyright (c) 2017 Michael T. Mayers

#include "mos6502.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t memory[65536];

void busWrite( uint16_t addr, uint8_t data )
{
	printf( RED "(%04x:%02x)" RESET, addr, data );
	memory[addr] = data;
}
uint8_t busRead( uint16_t addr )
{
	uint8_t data = memory[addr];
	printf( GREEN "(%04x:%02x)" RESET, addr, data );
	return data;
}


std::string statusstring(uint8_t st)
{
    std::string rval = "";
    if(st&1)
        rval += "C";
    if(st&2)
        rval += "Z";
    if(st&4)
        rval += "I";
    if(st&8)
        rval += "D";
    if(st&16)
        rval += "B";
    if(st&64)
        rval += "V";
    if(st&128)
        rval += "S";
    return rval;
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

    printf( "Performing CPU reset\n");
	mos6502 cpu(busRead,busWrite);
	cpu.Reset();
    printf( "\nCPU reset done..\n");


	for( int i=0; i<256; i++ )
	{
		auto PC = cpu.pc;
		auto SP = cpu.sp;
		auto X = cpu.X;
		auto Y = cpu.Y;
		auto A = cpu.A;
		auto ST = statusstring(cpu.status);

        static uint8_t LSP = SP;
        static auto LST = ST;
        static uint8_t LX = X;
        static uint8_t LY = Y;
        static uint8_t LA = A;

        bool chg_sp = LSP!=SP;
        bool chg_st = LST!=ST;
        bool chg_a = LA!=A;
        bool chg_x = LX!=X;
        bool chg_y = LY!=Y;


        LST = ST;
        LSP = SP;
        LA = A;
        LX = X;
        LY = Y;

		std::string statusline;


		statusline += Format( HIL "pc:" NUMCOL "%04X", PC );

        if( chg_sp )
    		statusline += Format( RED " sp:" CHGCOL "%02X", SP );
        else
            statusline += Format( HIL " sp:" NUMCOL "%02X", SP );

        if( chg_a )
    		statusline += Format( RED " a:" CHGCOL "%02X", A );
        else
            statusline += Format( HIL " a:" NUMCOL "%02X", A );

		if( chg_x )
            statusline += Format( RED " x:" CHGCOL "%02X", X );
        else
            statusline += Format( HIL " x:" NUMCOL "%02X", X );

        if( chg_y )
    		statusline += Format( RED " y:" CHGCOL "%02X", Y );
        else
            statusline += Format( HIL " y:" NUMCOL "%02X", Y );

        if( chg_st )
    		statusline += Format( RED " st:" CHGCOL "[%s]", ST.c_str() );
	   else
            statusline += Format( HIL " st:" NUMCOL "[%s]", ST.c_str() );

    	printf( "%s ", statusline.c_str() );
		cpu.Run(1);
	}
	return 0;
}