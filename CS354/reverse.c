void Reverse (char *str) {
	int i = 0;
	char* ps;
	char* pe;

	ps = str;

	while (*str != '\0') {
		i++;
		str++;
	}
	pe = ps + i - 1;

	for (int j = 0; j < i; j++) {
		int temp = *ps;
		*ps = *pe;
		*pe = temp;

		ps++;
		pe--;
		i--;
	}
}
