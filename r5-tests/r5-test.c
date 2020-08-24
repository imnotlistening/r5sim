/*
 * Simple program to verify the instruction decoding.
 */

const char *some_const_str = "Hello world";

unsigned int global = 0x1234567;

int hello(int arg1, const char *arg2[])
{
	unsigned int x = 1;
	unsigned int y = 2;
	unsigned int z = 1024*1024;

	arg1 += 10u;
	arg2[arg1] = some_const_str;

	return x + y + z;
}

void _start(void)
{
	int x = global;
	hello(1, (void *)x);
}
