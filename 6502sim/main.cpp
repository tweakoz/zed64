// 6502 sim driver
// copyright (c) 2017 Michael T. Mayers

#include "mos6502.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <ork/thread.h>
#include <GLFW/glfw3.h>

extern spsc_bounded_queue<uicmd> _uiQ;

uint8_t memory[65536];

void runUI();
std::set<uint16_t> addr_read_set;
std::set<uint16_t> addr_write_set;

void busWrite( uint16_t addr, uint8_t data )
{
	memory[addr] = data;
    addr_write_set.insert(addr);
}
uint8_t busRead( uint16_t addr )
{
	uint8_t data = memory[addr];
    addr_read_set.insert(addr);
	return data;
}
uint8_t busReadNT( uint16_t addr )
{
    uint8_t data = memory[addr];
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

    bool OKTOQUIT = false;

    ork::thread cputhread;
    cputhread.start([&](){
        printf( "Performing CPU reset\n");
        mos6502 cpu(busRead,busWrite);
        cpu.Reset();
        printf( "\nCPU reset done..\n");
        int runcount = 1;

        while(false==OKTOQUIT)
        {
            addr_read_set.clear();
            addr_write_set.clear();

            cpu.Run(runcount);

            bool cont = false;
            while(false==cont)
            {

                auto uic = _uiQ.pop();
                int mod = uic._mods;
                bool isshift = mod&GLFW_MOD_SHIFT;
                bool isalt = mod&GLFW_MOD_ALT;

                switch( uic._key )
                {
                    case GLFW_KEY_ESCAPE:
                        OKTOQUIT=true;
                        cont = true;
                        break;
                    case GLFW_KEY_SPACE:
                        cont = true;
                        runcount = isshift?4:1;
                        break;
                    case GLFW_KEY_ENTER:
                        cont = true;
                        runcount = isshift?(isalt?10000:100)
                                          :10;
                        break;
                    default:
                        break;
                }
            }
        }
    });

    runUI();
	return 0;
}