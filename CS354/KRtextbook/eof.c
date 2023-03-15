#include <stdio.h>

int main()
{
	int c;
	while ((c = getchar()) != EOF) {
		printf("%d", c);
		putchar(c);
	}
	return 0;
}
