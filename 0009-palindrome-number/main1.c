#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

bool
isPalindromeDigits(int x)
{
	if (x < 0)
		return false;
	if (x == 0)
		return true;
	int digits[12];
	int count = 0;
	while (x > 0) {
		digits[count++] = x % 10;
		x /= 10;
	}
	int middle = count / 2;
	int last = count - 1;
	for (int i = 0; i < middle; ++i) {
		if (digits[i] != digits[last - i])
			return false;
	}
	return true;
}

bool
isPalindromeReverse(int x)
{
	if (x < 0)
		return false;
	if (x == 0)
		return true;
	int64_t rev = 0;
	int64_t orig = x;
	while (x > 0) {
		rev = rev * 10 + x % 10;
		x /= 10;
	}
	return rev == orig;
}

// A very strange solution provided online. I didn't come up with it myself.
bool
isPalindromeReverseHalf(int x)
{
	if (x < 0)
		return false;
	if (x == 0)
		return true;
	if (x % 10 == 0)
		return false;
	int rev_half = 0;
	while (x > rev_half) {
		rev_half = rev_half * 10 + x % 10;
		x /= 10;
	}
	return rev_half == x || rev_half / 10 == x;
}

static void
check(bool ok)
{
	if (!ok)
		abort();
}

static void
test(bool (*func)(int))
{
	check(func(0));
	check(func(1));
	check(!func(10));
	check(func(11));
	check(func(121));
	check(func(1221));
	check(func(12321));
	check(!func(1211));
	check(!func(-1));
	check(!func(INT_MAX));
}

int
main()
{
	test(isPalindromeReverseHalf);
	test(isPalindromeDigits);
	test(isPalindromeReverse);
	return 0;
}
