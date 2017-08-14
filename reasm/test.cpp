auto TEST_PTR = (volatile char*) 0xd800;

int main(int argc, const char** argv)
{
    for( unsigned char i=0; i<40; i++ )
    {
        TEST_PTR[i] = i;
    }
    return 0;
}