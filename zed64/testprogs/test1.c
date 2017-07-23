int main()
{
	const int kstrlen = 12;
	const char str[12] = "hello world";

	char* dest = (char*) 0x4000;

	int index = 0;
    char value = 0;

	while(1)
	{
        dest[index++] = value++;
	}
	return 0;
}