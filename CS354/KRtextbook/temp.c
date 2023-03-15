#include <stdio.h>

#define LOWER  0     /* lower limit of table */
#define UPPER 100   /* upper limit of table */
#define STEP   5     /* step size */

int main()
{
	float fahr, celsius;
	int lower, upper, step;

	lower = LOWER;
	upper = UPPER;
	step = STEP;

	celsius = lower;

	printf("Celcius to Fahrenheit\n");
	while (celsius <= upper) {
		fahr = (celsius * 9.0 / 5.0) + 32;
		printf("%3.0f %6.2f\n", celsius, fahr);
		celsius = celsius + step;
	}

	printf("Fahrenheit to Celcius\n");
	for (int fahr = 300; fahr >= 0; fahr = fahr - 20) {
		printf("%3d %6.2f\n", fahr, (5.0/9.0)*(fahr-32));
	}
}
