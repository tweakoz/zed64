int main()
{
	const int kstrlen = 12;
	const char str[12] = "hello world";

	char* dest = (char*) 0x4000;

	int addr = 0;
	while(1)
	{
		addr = (addr+11)%1000;
	}
	return 0;
}