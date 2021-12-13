#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

static int
int_cmp(const void *l, const void *r)
{
	return *(const int *)l - *(const int *)r;
}

int
threeSumClosest(int *nums, int count, int target)
{
	assert(count >= 3);
	int res = nums[0] + nums[1] + nums[2];
	if (res == target)
		return target;

	int min_diff;
	if (res >= target)
		min_diff = res - target;
	else
		min_diff = target - res;

	// The algorithm is to select elements one by one and then perform 2Sum
	// on the other elements to find the reversed number. In order for that
	// to be working the array needs to be sorted.
	// Total complexity is quadratic. And sorting is not the worst part of
	// it.

	qsort(nums, count, sizeof(*nums), int_cmp);

	int i1 = 0;
	for (int i1 = 0; i1 < count - 2; ++i1) {
		if (i1 > 0 && nums[i1] == nums[i1 - 1])
			continue;
		int l = i1 + 1;
		int r = count - 1;
		int sum1 = nums[i1];
		while (l < r) {
			int sum3 = nums[l] + nums[r] + sum1;
			int diff;
			if (sum3 >= target)
				diff = sum3 - target;
			else
				diff = target - sum3;
			if (diff < min_diff) {
				if (diff == 0)
					return target;
				min_diff = diff;
				res = sum3;
			}
			if (sum3 < target) {
				++l;
				continue;
			} else if (sum3 > target) {
				--r;
				continue;
			}
			++l;
			--r;
			while (l < r && nums[l] == nums[l - 1])
				++l;
			while (l < r && nums[r] == nums[r + 1])
				--r;
		}
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
	{
		int nums[] = {-3, -2, -5, 3, -4};
		check(threeSumClosest(nums, 5, -1) == -2);
	}
	{
		int nums[] = {-1, 2, 1, -4};
		check(threeSumClosest(nums, 4, 1) == 2);
	}
	{
		int nums[] = {0, 0, 0};
		check(threeSumClosest(nums, 3, 1) == 0);
	}
	{
		int nums[] = {-1, 0, 1, 2, -1, -4};
		check(threeSumClosest(nums, 6, 0) == 0);
	}
	{
		int nums[] = {-1, 0, 1};
		check(threeSumClosest(nums, 3, 0) == 0);
	}
	{
		int nums[] = {-2, 0, 1, 1, 2};
		check(threeSumClosest(nums, 5, 0) == 0);
	}
	return 0;
}
