#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

enum {
	MAX_VALUE_LIMIT = 2000,
	CACHE_STATIC_SIZE = 10,
};

// Use numbers as indexes. Works only while they are non-negative.
static void
big_sort_brute_force(int *arr, int count, int max_value)
{
	assert(max_value >= 0 && max_value <= MAX_VALUE_LIMIT);
	int counts[MAX_VALUE_LIMIT];
	memset(counts, 0, (max_value + 1) * sizeof(counts[0]));
	for (int i = 0; i < count; ++i) {
		int item = arr[i];
		assert(item >= 0 && item <= max_value);
		++counts[arr[i]];
	}
	for (int arr_i = 0, i = 0; i <= max_value; ++i) {
		int one_count = counts[i];
		for (int j = 0; j < one_count; ++j, ++arr_i) {
			assert(arr_i < count);
			arr[arr_i] = i;
		}
	}
}

// Sort-sliding window. It can work for negative numbers too, but not required.
// The idea is to use the fact that for each item arr[i] the only values out of
// order afterwards can be integers in the range [arr[i] - sort_factor, arr[i]].
//
// The solution is to store counters just for values in this range. Not for all
// the values in [0, max_value] like in the brute force solution. When arr[i]
// grows, this sliding window must be moved. The items falling out of it are
// flushed into the result array.
//
// Moreover, just storing the counters in an array and moving it by memmove()
// might be too expensive if it moves often and the sort factor is big. To
// overcome the issue the solution uses a cyclic buffer. That way the window
// move operation costs linear from move width.
static void
big_sort_stream(int *arr, int count, int sort_factor)
{
	if (count == 0)
		return;
	// Use on-stack buffer when possible. To avoid excessive heap usage.
	int cache_static[CACHE_STATIC_SIZE];
	int *cache;
	int cache_size = sort_factor + 1;
	if (cache_size <= CACHE_STATIC_SIZE)
		cache = cache_static;
	else
		cache = malloc(sizeof(cache[0]) * cache_size);
	memset(cache, 0, sizeof(cache[0]) * cache_size);
	// The max known value is in the end of the cache. The out of order
	// smaller values have counters on the left side.
	int cache_base = arr[0];
	int cache_end_idx = cache_size;
	cache[0] = 1;

	int res_i = 0;
	int cache_start_idx;
	int start_value;
	for (int i = 1; i < count; ++i) {
		int item = arr[i];
		if (item <= cache_base) {
			// An out of order value. It must not be different more
			// than the sort_factor.
			int idx = cache_end_idx - (cache_base - item);
			assert(idx >= 0);
			++cache[idx % cache_size];
			continue;
		}
		int diff = item - cache_base;
		int to_cut = diff;
		// If the leap was too big, it wouldn't make sense to walk the
		// same buffer multiple times. Worst case - walk and dump it
		// whole only once.
		if (to_cut > cache_size)
			to_cut = cache_size;
		cache_start_idx = cache_end_idx - cache_size + 1;
		start_value = cache_base - cache_size + 1;
		for (int j = 0; j < to_cut; ++j) {
			int idx = (cache_start_idx + j) % cache_size;
			int value = start_value + j;
			int dump_count = cache[idx];
			if (dump_count == 0)
				continue;
			cache[idx] = 0;
			for (int k = 0; k < dump_count; ++k)
				arr[res_i++] = value;
		}
		cache_base = item;
		cache_end_idx += diff;
		cache[cache_end_idx % cache_size] = 1;
	}
	// Flush whatever is left.
	cache_start_idx = cache_end_idx - cache_size + 1;
	start_value = cache_base - cache_size + 1;
	for (int j = 0; j < cache_size; ++j) {
		int idx = (cache_start_idx + j) % cache_size;
		int value = start_value + j;
		int dump_count = cache[idx];
		if (dump_count == 0)
			continue;
		for (int k = 0; k < dump_count; ++k)
			arr[res_i++] = value;
	}
	if (cache != cache_static)
		free(cache);
}

static void
big_sort(int *arr, int count, int max_value, int sort_factor)
{
	if (sort_factor >= max_value)
		big_sort_brute_force(arr, count, max_value);
	else
		big_sort_stream(arr, count, sort_factor);
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static int
int_cmp_cb(const void *a, const void *b)
{
	return *(const int *)a - *(const int *)b;
}

static int *
array_copy(const int *src, int count)
{
	int *res = malloc(count * sizeof(res[0]));
	memcpy(res, src, count * sizeof(res[0]));
	return res;
}

static bool
array_eq(const int *arr1, const int *arr2, int count)
{
	return memcmp(arr1, arr2, count * sizeof(arr1[0])) == 0;
}

static inline void
big_sort_test(const int *arr, int count, int max_value, int sort_factor)
{
	int *expected = array_copy(arr, count);
	qsort(expected, count, sizeof(expected[0]), int_cmp_cb);

	int *arr2 = array_copy(arr, count);
	big_sort_stream(arr2, count, sort_factor);
	check(array_eq(arr2, expected, count));
	free(arr2);

	arr2 = array_copy(arr, count);
	big_sort_brute_force(arr2, count, max_value);
	check(array_eq(arr2, expected, count));
	free(arr2);

	arr2 = array_copy(arr, count);
	big_sort(arr2, count, max_value, sort_factor);
	check(array_eq(arr2, expected, count));
	free(arr2);

	free(expected);
}

int
main()
{
	{
		const int arr[] = {0, 1, 2, 3};
		big_sort_test(arr, 4, 3, 0);
	}
	{
		const int arr[] = {0, 1, 2, 3};
		big_sort_test(arr, 4, 3, 100);
	}
	{
		const int arr[] = {0, 1, 2, 9, 19};
		big_sort_test(arr, 5, 19, 1);
	}
	{
		const int arr[] = {13, 8, 3, 4, 5, 2, 1, 0};
		big_sort_test(arr, 8, 13, 20);
	}
	{
		big_sort_test(NULL, 0, 0, 5);
	}
	{
		const int arr[] = {1};
		big_sort_test(arr, 1, 1, 5);
	}
	{
		const int arr[] = {2, 1, 3, 2, 4, 3, 6, 5, 8, 7};
		big_sort_test(arr, 10, 8, 1);
	}
	{
		const int arr[] = {2, 1, 3, 2, 4, 3, 6, 5, 8, 7};
		big_sort_test(arr, 10, 8, 2);
	}
	{
		const int arr[] = {5, 2, 4, 1, 3, 9, 6, 10, 8, 7, 110, 106,
			108, 120};
		big_sort_test(arr, 14, 120, 4);
	}
	return 0;
}
