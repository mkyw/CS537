#include <stdio.h>

float conv (int temp);

int main()
{
	int l = 0;
	int u  = 300;

	printf("Fahrenheit to Celcius\n");

	for (l = 0; l <= u; l = l + 20) {
		printf("%3d %6.2f\n", l, conv(l));
	}
	return 0;
}

float conv (int temp) {
	return (temp - 32.0) * (5.0 / 9.0);
}
