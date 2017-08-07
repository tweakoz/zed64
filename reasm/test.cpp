auto TEST_PTR = (volatile char*) 0xd800;

int main(int argc, const char** argv)
{
    for( int i=0; i<40; i++ )
    {
        TEST_PTR[i] = 1;
    }
    return 0;
}