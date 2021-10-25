#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

double
findMedianSortedArrays(
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

int
main()
{
	{
		int arr1[] = {1, 3};
		int arr2[] = {2};
		printf("%lf\n", findMedianSortedArrays(arr1, 2, arr2, 1));
	}
	{
		int arr1[] = {1, 2};
		int arr2[] = {3, 4};
		printf("%lf\n", findMedianSortedArrays(arr1, 2, arr2, 2));
	}
	{
		int arr1[] = {0, 0};
		int arr2[] = {0, 0};
		printf("%lf\n", findMedianSortedArrays(arr1, 2, arr2, 2));
	}
	{
		int arr2[] = {1};
		printf("%lf\n", findMedianSortedArrays(NULL, 0, arr2, 1));
	}
	{
		int arr1[] = {2};
		printf("%lf\n", findMedianSortedArrays(arr1, 1, NULL, 0));
	}
	return 0;
}
