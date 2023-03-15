#include <stdio.h>

int main()
{
	int c, b, t, nl;
	b = 0;
	t = 0;
	nl = 0;

	while ((c = getchar()) != EOF) {
		if (c == ' ')
			++b;
		else if (c == '\t')
			++t;
		else if (c == '\n')
			++nl;
	}
	printf("Blanks: %d, Tabs: %d, New lines: %d\n", b, t, nl);
	
	while ((c = getchar()) != EOF) {
		if (c == '\t') {
			printf("\\t");
		} else if (c == '\b') {
			printf("\\b");
		} else if (c == '\\') {
			printf("\\\\");
		} else {
			putchar(c);
		}
	}
	printf("\n");
	return 0;
}
	
