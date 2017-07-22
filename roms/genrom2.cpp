#include <stdio.h>
#include <string>

int main( int argc, const char** argv )
{
    std::string outstr;

	for( int i=0; i<512; i++ )
        outstr = outstr + "helloworld";

    outstr.resize(4095);

    for( int i=0; i<4095; i++ )
    {
        putchar(outstr[i]-'a'+1);
    }
    putchar(0);
	return 0;
}
