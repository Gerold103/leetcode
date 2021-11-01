#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

int
reverse(int x)
{
	int sign;
	if (x < 0) {
		if (x == INT_MIN)
			return 0;
		sign = -1;
		x *= -1;
	} else {
		sign = 1;
	}
	int res = 0;
	while (x != 0) {
		int d = x % 10;
		if (res > (INT_MAX - d) / 10)
			return 0;
		res = res * 10 + d;
		x /= 10;
	}
	return res * sign;
}

int
main()
{
	printf("%d\n", reverse(123));
	printf("%d\n", reverse(321));
	printf("%d\n", reverse(1));
	printf("%d\n", reverse(0));
	printf("%d\n", reverse(-123));
	printf("%d\n", reverse(-1));
	printf("%d\n", reverse(2147483647));
	return 0;
}
