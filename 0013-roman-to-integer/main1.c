#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

int
romanToInt(const char *s)
{
	const int digits[] = {
		['I'] = 1,
		['V'] = 5,
		['X'] = 10,
		['L'] = 50,
		['C'] = 100,
		['D'] = 500,
		['M'] = 1000,
	};
	const char *pos = s;
	if (*pos == 0)
		return 0;
	int res = 0;
	int d_cur;
	int d_next = digits[*pos];
	while (true) {
		d_cur = d_next;
		++pos;
		if (*pos == 0) {
			res += d_cur;
			break;
		}
		d_next = digits[*pos];
		if (d_cur >= d_next)
			res += d_cur;
		else
			res -= d_cur;
	}
	return res;
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

int
main()
{
	check(romanToInt("III") == 3);
	check(romanToInt("") == 0);
	check(romanToInt("IV") == 4);
	check(romanToInt("IX") == 9);
	check(romanToInt("LVIII") == 58);
	check(romanToInt("MCMXCIV") == 1994);
	return 0;
}
