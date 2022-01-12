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
fourSum(int *nums, int count, int target, int *res_count_out,
	int **item_count_out)
{
	int res_cap = 0;
	int res_count = 0;
	int *item_count = NULL;
	int **res = NULL;

	// The algorithm is to select elements one by one and then perform 2Sum
	// on the other elements to find the reversed number. In order for that
	// to be working the array needs to be sorted.

	qsort(nums, count, sizeof(*nums), int_cmp);

	// It can be converted to a more generic kSum via recursion. But this
	// solution was just easier to do.
	int i1 = 0;
	for (int i1 = 0; i1 < count - 3; ++i1) {
		if (i1 > 0 && nums[i1] == nums[i1 - 1])
			continue;
		for (int i2 = i1 + 1; i2 < count - 2; ++i2) {
			if (i2 > i1 + 1 && nums[i2] == nums[i2 - 1])
				continue;
			int l = i2 + 1;
			int r = count - 1;
			int tmp_target = target - nums[i1] - nums[i2];
			while (l < r) {
				int sum = nums[l] + nums[r];
				if (sum < tmp_target) {
					++l;
					continue;
				} else if (sum > tmp_target) {
					--r;
					continue;
				}
				int new_count = res_count + 1;
				int *quadriplet = malloc(4 * sizeof(**res));
				quadriplet[0] = nums[i1];
				quadriplet[1] = nums[i2];
				quadriplet[2] = nums[l];
				quadriplet[3] = nums[r];

				// Without x2 capacity growth it does not
				// pass online tests in time.
				if (res_count == res_cap) {
					res_cap = (res_cap + 1) * 2;
					item_count = realloc(item_count,
						res_cap * sizeof(*item_count));
					res = realloc(res, res_cap * sizeof(*res));
				}
				item_count[res_count] = 4;
				res[res_count] = quadriplet;
				res_count = new_count;
				++l;
				--r;
				while (l < r && nums[l] == nums[l - 1])
					++l;
				while (l < r && nums[r] == nums[r + 1])
					--r;
			}
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
check_res(int *nums, int count, int target, const int **expected, int expected_count)
{
	int res_count;
	int *item_count;
	int **res = fourSum(nums, count, target, &res_count, &item_count);
	check(res_count == expected_count);
	for (int i = 0; i < res_count; ++i) {
		check(item_count[i] == 4);
		for (int j = 0; j < 4; ++j)
			check(expected[i][j] == res[i][j]);
		free(res[i]);
	}
	free(res);
}

int
main()
{
	{
		int nums[] = {1, 0, -1, 0, -2, 2};
		int expected1[] = {-2, -1, 1, 2};
		int expected2[] = {-2, 0, 0, 2};
		int expected3[] = {-1, 0, 0, 1};
		const int *expected[] = {expected1, expected2, expected3};

		check_res(nums, 6, 0, expected, 3);
	}
	{
		int nums[] = {2, 2, 2, 2, 2};
		int expected1[] = {2, 2, 2, 2};
		const int *expected[] = {expected1};

		check_res(nums, 5, 8, expected, 1);
	}
	return 0;
}
