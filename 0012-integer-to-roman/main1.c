#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

static void
roman_sprintf_order(char **pos_arg, int *n_arg, int one, char one_sym, int five,
		    char five_sym, int ten, char ten_sym)
{
	char *pos = *pos_arg;
	int n = *n_arg;
	int four = five - one;
	if (n < four) {
		int count = n / one;
		memset(pos, one_sym, count);
		pos += count;
		n -= one * count;
		goto end;
	}
	if (n < five) {
		*(pos++) = one_sym;
		*(pos++) = five_sym;
		n -= four;
		goto end;
	}
	int nine = ten - one;
	if (n < nine) {
		*(pos++) = five_sym;
		n -= five;
		int count = n / one;
		memset(pos, one_sym, count);
		pos += count;
		n -= one * count;
		goto end;
	}
	if (n < ten) {
		*(pos++) = one_sym;
		*(pos++) = ten_sym;
		n -= nine;
		goto end;
	}
	*(pos++) = ten_sym;
	n -= ten;
end:
	*pos_arg = pos;
	*n_arg = n;
}

char *
intToRomanOpt1(int n)
{
	assert(n < 4000 && n > 0);
	char res[32];
	char *pos = res;

	roman_sprintf_order(&pos, &n, 1000, 'M', 5000, '*', 10000, '*');
	roman_sprintf_order(&pos, &n, 100, 'C', 500, 'D', 1000, 'M');
	roman_sprintf_order(&pos, &n, 10, 'X', 50, 'L', 100, 'C');
	roman_sprintf_order(&pos, &n, 1, 'I', 5, 'V', 10, 'X');

	*pos = 0;
	uint32_t size = pos - res + 1;
	char *buf = malloc(size);
	memcpy(buf, res, size);
	return buf;
}

struct digit {
	int val;
	const char *sym;
	int sym_len;
};

char *
intToRomanOpt2(int n)
{
	assert(n < 4000 && n > 0);
	char res[32];
	char *pos = res;

	const struct digit digits[] = {
		{1000, "M", 1}, {900, "CM", 2}, {500, "D", 1}, {400, "CD", 2},
		{100, "C", 1}, {90, "XC", 2}, {50, "L", 1}, {40, "XL", 2},
		{10, "X", 1}, {9, "IX", 2}, {5, "V", 1}, {4, "IV", 2},
		{1, "I", 1}, {0, NULL}
	};
	const struct digit *d = digits;
	while (n > 0) {
		while (n >= d->val) {
			memcpy(pos, d->sym, d->sym_len);
			pos += d->sym_len;
			n -= d->val;
		}
		++d;
	}

	*pos = 0;
	uint32_t size = pos - res + 1;
	char *buf = malloc(size);
	memcpy(buf, res, size);
	return buf;
}

static void
check_str(char *str, const char *expected)
{
	if (strcmp(str, expected) != 0)
		abort();
	free(str);
}

static void
test(char *(*func)(int))
{
	check_str(func(3), "III");
	check_str(func(4), "IV");
	check_str(func(9), "IX");
	check_str(func(58), "LVIII");
	check_str(func(1994), "MCMXCIV");

}

int
main()
{
	test(intToRomanOpt1);
	test(intToRomanOpt2);
	return 0;
}
