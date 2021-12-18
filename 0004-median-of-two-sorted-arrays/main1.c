#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

double
findMedianSortedArraysLinear(
	const int* nums1, int nums1Size,
	const int* nums2, int nums2Size)
{
	if (nums1Size > nums2Size) {
		int tmpi = nums1Size;
		nums1Size = nums2Size;
		nums2Size = tmpi;

		const int* tmpp = nums1;
		nums1 = nums2;
		nums2 = tmpp;
	}

	int total = nums1Size + nums2Size;
	int med_idx2 = total / 2;
	int med_idx1;
	if (total % 2 == 0)
		med_idx1 = med_idx2 - 1;
	else
		med_idx1 = med_idx2;

	// Skip unnecessary elements of both arrays.
	int fast_count = med_idx1 - 1;
	if (fast_count > nums1Size)
		fast_count = nums1Size;
	int i1 = 0;
	int i2 = 0;
	int i = 0;
	for (; i < fast_count; ++i) {
		if (nums1[i1] > nums2[i2])
			++i2;
		else
			++i1;
	}

	// Skip elements where don't need to know median yet.
	for (; i < med_idx1; ++i) {
		if (i1 >= nums1Size) {
			i2 += med_idx1 - i;
			break;
		}
		else if (i2 >= nums2Size) {
			i1 += med_idx1 - i;
			break;
		}
		if (nums1[i1] > nums2[i2])
			++i2;
		else
			++i1;
	}

	// Get the first part of the median.
	int value1;
	if (i1 >= nums1Size)
		value1 = nums2[i2++];
	else if (i2 >= nums2Size)
		value1 = nums1[i1++];
	else if (nums1[i1] > nums2[i2])
		value1 = nums2[i2++];
	else
		value1 = nums1[i1++];

	if (med_idx2 == med_idx1)
		return value1;

	// Get the second part of the median.
	int value2;
	if (i1 >= nums1Size)
		value2 = nums2[i2];
	else if (i2 >= nums2Size)
		value2 = nums1[i1];
	else if (nums1[i1] > nums2[i2])
		value2 = nums2[i2];
	else
		value2 = nums1[i1];
	return (value1 + value2) / 2.0;
}

static double
findMedianSortedArray(
	const int* nums, int numsSize)
{
	assert(numsSize > 0);
	int middle = numsSize / 2;
	if (numsSize % 2 != 0)
		return nums[middle];
	return (nums[middle] + nums[middle - 1]) / 2.0;
}

double
findMedianSortedArrays(
	const int* nums1, int nums1Size,
	const int* nums2, int nums2Size)
{
	if (nums1Size == 0)
		return findMedianSortedArray(nums2, nums2Size);
	if (nums2Size == 0)
		return findMedianSortedArray(nums1, nums1Size);

	// The solution is based on median value definition. A median value in
	// a sorted array 'arr' has index 'i' if:
	//
	// 1) i == len / 2. If it is odd, then median would also include
	//    arr[i - 1].
	//
	// 2) arr[i-1] <= arr[i..len].
	//
	// If there are 2 arrays, then assume their medians are at indexes i1
	// and i2 correspondingly:
	//
	//   arr1: [    m1    ]
	//   arr2: [        m2         ]
	//
	// Median of a merged array will be on the position (len1 + len2) / 2.
	// Or otherwise: m1 + m2. It is the same.
	//
	// Also at that position we can reword the (2) point as a new point (3):
	//
	// 3) max(arr1[m1-1], arr2[m2-1]) <= min(arr1[m1], arr2[m2]).
	//
	// The task here is to find such m1 and m2 that the condition above
	// works.
	//
	// Obviously, trying all possible combinations of m1 and m2 is not an
	// option. But it is possible to try various m1 and get as m2 = f(m1).
	// Indeed, they are dependent. Their sum must be (len1 + len2) / 2. So
	// as merged array's index m1+m2 would be in the middle.
	//
	// Then we can try binary search on m1 in [0..len1] range. On each step
	// we use the formula:
	//
	//   m1 + m2 == (len1 + len2) / 2, hence
	//   m2 == (len1 + len2) / 2 - m1
	//
	// On each step of the binary search we check if (3) is satisfied. If it
	// isn't, then look what is wrong. Only 4 options are possible:
	//
	//   arr1[m1-1] > arr1[m1] - impossible in a sorted array arr1.
	//   arr1[m1-1] > arr2[m2]
	//   arr2[m2-1] > arr1[m1]
	//   arr2[m2-1] > arr2[m2]  - impossible in a sorted array arr2.
	//
	// Only 2 options are possible. If arr1[m1-1] is too big, then reduce
	// m1. Otherwise increase m1. That is the binary search step.
	//

	if (nums1Size > nums2Size) {
		// Iteration always on the smaller array helps to ensure the
		// index of the second array will never overflow. Indeed,
		// consider how i2 is calculated on each step:
		//
		//   i2 = (len1 + len2) / 2 - i1
		//
		// i2 gets maximal value when i1 = 0. It means this is true:
		//
		//   i2 <= (len1 + len2) / 2
		//
		// If len2 is >= len1, then this is true as well:
		//
		//   i2 <= (len2 + len2) / 2
		//
		// And this turns into:
		//
		//   i2 <= len2
		//
		// The case of i2 == len2 is handled. i2 > len2 is not possible,
		// don't need to handle it.
		//
		const int* tmpPtr = nums1;
		nums1 = nums2;
		nums2 = tmpPtr;

		int tmp = nums1Size;
		nums1Size = nums2Size;
		nums2Size = tmp;
	}

	int left = 0;
	int right = nums1Size;
	int lenSum = nums1Size + nums2Size;
	int lenSumHalf = lenSum / 2;
	int part1 = -1, part2 = -1;
	assert(left <= right);
	do {
		// Find candidate for median index. So as
		//
		//   i1 + i2 == (len1 + len2) / 2.
		//
		int i1 = left + (right - left) / 2;
		int i2 = lenSumHalf - i1;
		assert(i1 >= 0);
		assert(i2 >= 0);
		assert(i1 <= nums1Size);
		assert(i2 <= nums2Size);

		// Now check the value in this index is a median. All left
		// values <= all right values.
		//
		int num1_left = i1 == 0 ? INT_MIN : nums1[i1 - 1];
		int num2_left = i2 == 0 ? INT_MIN : nums2[i2 - 1];
		int max_left;
		if (num1_left <= num2_left)
			max_left = num2_left;
		else
			max_left = num1_left;

		int num1_right = i1 == nums1Size ? INT_MAX : nums1[i1];
		int num2_right = i2 == nums2Size ? INT_MAX : nums2[i2];
		int min_right;
		if (num1_right <= num2_right)
			min_right = num1_right;
		else
			min_right = num2_right;

		if (max_left <= min_right) {
			part1 = max_left;
			part2 = min_right;
			break;
		}

		if (num1_left > num2_right)
			right = i1 - 1;
		else
			left = i1 + 1;
	} while (left <= right);

	if (lenSum % 2 == 0)
		return (part1 + part2) / 2.0;

	return part2;
}

static void
test(double (*func)(const int*, int, const int*, int))
{
	{
		int arr1[] = {1, 3};
		int arr2[] = {2};
		printf("%lf\n", func(arr1, 2, arr2, 1));
	}
	{
		int arr1[] = {1, 2};
		int arr2[] = {3, 4};
		printf("%lf\n", func(arr1, 2, arr2, 2));
	}
	{
		int arr1[] = {0, 0};
		int arr2[] = {0, 0};
		printf("%lf\n", func(arr1, 2, arr2, 2));
	}
	{
		int arr2[] = {1};
		printf("%lf\n", func(NULL, 0, arr2, 1));
	}
	{
		int arr1[] = {2};
		printf("%lf\n", func(arr1, 1, NULL, 0));
	}
	{
		int arr1[] = {2, 3, 4, 5};
		int arr2[] = {1, 0, 0, 0};
		printf("%lf\n", func(arr1, 4, arr2, 1));
	}

}

int
main()
{
	printf("Linear\n");
	test(findMedianSortedArraysLinear);
	printf("\nLog\n");
	test(findMedianSortedArrays);
	return 0;
}
