#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

int
divide(int dividend, int divisor)
{
	// The idea is to use kind of a binary search on the range
	// of [0, dividend]. Being honest, I couldn't invent this
	// solution myself. The idea was taken from the Internet.
	// My main part is that I was able not to use 64bit
	// numbers at all. Also I managed to simplify corner case
	// checks. Essentially, they become not needed except for
	// the singe overflow case (INT_MIN/-1, checked in the
	// end) if use unsigned numbers instead of plain ints.
	int sign = 0;
	unsigned res = 0;
	unsigned udividend, udivisor;
	if (divisor < 0) {
		sign = 1;
		udivisor = -(unsigned)divisor;
	} else {
		udivisor = divisor;
	}
	if (dividend < 0) {
		++sign;
		udividend = -(unsigned)dividend;
	} else {
		udividend = dividend;
	}
	while (udividend >= udivisor) {
		unsigned sub = udivisor;
		unsigned mult = 1;
		// Otherwise if the dividend is
		// INT_MAX, on the last attempt to
		// find the border the sub can
		// overflow.
		while (sub <= (UINT_MAX >> 1)) {
			sub <<= 1;
			if (sub > udividend) {
				sub >>= 1;
				break;
			}
			mult <<= 1;
		};
		udividend -= sub;
		res += mult;
	}
	if (sign != 1) {
		if (res >= INT_MAX)
			return INT_MAX;
		return res;
	}
	return (int)-res;
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
	check(divide(15, 3) == 5);
	check(divide(1, 1) == 1);
	check(divide(0, 1) == 0);
	check(divide(12, 4) == 3);
	check(divide(4, 4) == 1);
	check(divide(6, 3) == 2);
	check(divide(17, 5) == 3);
	check(divide(14, 3) == 4);
	check(divide(0, -1) == 0);
	check(divide(-4, 2) == -2);
	check(divide(-4, -3) == 1);

	check(divide(INT_MAX, 1) == INT_MAX);
	check(divide(INT_MIN, -1) == INT_MAX);
	check(divide(INT_MIN, 1) == INT_MIN);
	check(divide(INT_MAX, -1) == -INT_MAX);
	check(divide(INT_MAX, INT_MIN) == 0);
	check(divide(INT_MIN, INT_MAX) == -1);
	check(divide(INT_MIN, -INT_MAX) == 1);
	check(divide(INT_MIN, INT_MIN) == 1);
	return 0;
}
