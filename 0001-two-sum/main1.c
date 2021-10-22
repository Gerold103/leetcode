#include <stdlib.h>
#include <stdio.h>

int*
twoSum(int* nums, int numsSize, int target, int* returnSize){
	for (int i = 0; i < numsSize; ++i) {
		for (int j = i + 1; j < numsSize; ++j) {
			if (nums[i] + nums[j] == target) {
				nums = malloc(sizeof(nums[0]) * 2);
				nums[0] = i;
				nums[1] = j;
				*returnSize = 2;
				return nums;
			}
		}
	}
	*returnSize = 0;
	return NULL;
}

static void
consume_arr(int *vals, int count)
{
	printf("[");
	for (int i = 0; i < count; ++i)
		printf("%s%d", i > 0 ? ", " : "", vals[i]);
	printf("]\n");
	free(vals);
}

int
main()
{
	int nums[4];
	int count;
	int *res;

	nums[0] = 1, nums[1] = 2, nums[2] = 3, nums[3] = 4;
	for (int i = 3; i <= 7; ++i) {
		res = twoSum(nums, 4, i, &count);
		consume_arr(res, count);
	}

	nums[0] = 3, nums[1] = 2, nums[2] = 4;
	res = twoSum(nums, 3, 6, &count);
	consume_arr(res, count);
	return 0;
}
