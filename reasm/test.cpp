auto CHAR_PTR = (volatile char*) 0xd000;
auto COLR_PTR = (volatile char*) 0xd800;

int main(int argc, const char** argv)
{
    unsigned char base = 0;
    for( unsigned short j=0; j<32; j++ )
    {
        int offset = j<<5;
        for( unsigned char i=0; i<32; i++ )
        {
            CHAR_PTR[offset+i] = base+i;
            COLR_PTR[offset+i] = base+i;
        }
    }
    return 0;
}