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

int **
threeSum(int *nums, int count, int *res_count_out, int **item_count_out)
{
	int res_cap = 0;
	int res_count = 0;
	int *item_count = NULL;
	int **res = NULL;

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
		int target = -nums[i1];
		if (target < 0)
			break;
		while (l < r) {
			int sum = nums[l] + nums[r];
			if (sum < target) {
				++l;
				continue;
			} else if (sum > target) {
				--r;
				continue;
			}
			int new_count = res_count + 1;
			int *triplet = malloc(3 * sizeof(**res));
			triplet[0] = nums[i1];
			triplet[1] = nums[l];
			triplet[2] = nums[r];

			// Without x2 capacity growth it does not
			// pass online tests in time.
			if (res_count == res_cap) {
				res_cap = (res_cap + 1) * 2;
				item_count = realloc(item_count,
					res_cap * sizeof(*item_count));
				res = realloc(res, res_cap * sizeof(*res));
			}
			item_count[res_count] = 3;
			res[res_count] = triplet;
			res_count = new_count;
			++l;
			--r;
			while (l < r && nums[l] == nums[l - 1])
				++l;
			while (l < r && nums[r] == nums[r + 1])
				--r;
		}
	}
	*res_count_out = res_count;
	*item_count_out = item_count;
	return res;
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static inline void
check_res(int *nums, int count, const int **expected, int expected_count)
{
	int res_count;
	int *item_count;
	int **res = threeSum(nums, count, &res_count, &item_count);
	check(res_count == expected_count);
	for (int i = 0; i < res_count; ++i) {
		check(item_count[i] == 3);
		for (int j = 0; j < 3; ++j)
			check(expected[i][j] == res[i][j]);
		free(res[i]);
	}
	free(res);
}

int
main()
{
	{
		int nums[] = {-1, 0, 1, 2, -1, -4};
		int expected1[] = {-1, -1, 2};
		int expected2[] = {-1, 0, 1};
		const int *expected[] = {expected1, expected2};

		check_res(nums, 6, expected, 2);
	}
	{
		check_res(NULL, 0, NULL, 0);
	}
	{
		int nums[] = {0};
		check_res(nums, 1, NULL, 0);
	}
	{
		int nums[] = {-1, 0, 1};
		const int *expected[] = {nums};

		check_res(nums, 3, expected, 1);
	}
	{
		int nums[] = {-2, 0, 1, 1, 2};
		int expected1[] = {-2, 0, 2};
		int expected2[] = {-2, 1, 1};
		const int *expected[] = {expected1, expected2};

		check_res(nums, 5, expected, 2);
	}
	return 0;
}
