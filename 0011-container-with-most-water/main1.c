#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

int
maxAreaBruteForce(const int *heights, int count)
{
	int last = count - 1;
	int max = 0;
	for (int i = 0; i < last; ++i) {
		int l = heights[i];
		for (int j = i + 1; j < count; ++j) {
			int r = heights[j];
			int h = r <= l ? r : l;
			int w = j - i;
			int size = h * w;
			if (size > max)
				max = size;
		}
	}
	return max;
}

int
maxAreaSkipLess(const int *heights, int count)
{
	int last = count - 1;
	int max = 0;
	int max_l = 0;
	for (int i = 0; i < last; ++i) {
		int l = heights[i];
		if (l <= max_l)
			continue;
		max_l = l;
		int max_r = 0;
		for (int j = last; j > i; --j) {
			int r = heights[j];
			if (r <= max_r)
				continue;
			max_r = r;
			int h = r <= l ? r : l;
			int w = j - i;
			int size = h * w;
			if (size > max)
				max = size;
		}
	}
	return max;
}

int
maxAreaLinear(const int *heights, int count)
{
	int last = count - 1;
	const int *l = heights;
	const int *r = heights + last;
	int lh = *l;
	int rh = *r;
	int w = last;
	int max = 0;
	int v;
	while (l < r) {
		if (lh < rh) {
			v = lh;
			lh = *++l;
		} else {
			v = rh;
			rh = *--r;
		}
		v *= w--;
		if (v > max)
			max = v;
	}
	return max;
}

static void
check(bool ok)
{
	if (!ok)
		abort();
}

static void
test(int (*func)(const int *heights, int count))
{
	check(func((int[]){1, 1}, 2) == 1);
	check(func((int[]){4, 3, 2, 1, 4}, 5) == 16);
	check(func((int[]){1, 2, 1}, 3) == 2);
	check(func((int[]){1, 8, 6, 2, 5, 4, 8, 3, 7}, 9) == 49);
}

int
main()
{
	test(maxAreaBruteForce);
	test(maxAreaSkipLess);
	test(maxAreaLinear);
	return 0;
}
