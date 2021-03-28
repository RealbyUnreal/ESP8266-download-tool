#include <stdio.h>
#define REG 0x000000F0

int main()
{
	int x = (0x00000020 & REG) >> 5;
	printf("%d", x);
}