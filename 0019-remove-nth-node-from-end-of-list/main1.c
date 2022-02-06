#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

struct ListNode {
	int val;
	struct ListNode *next;
};

struct ListNode *
removeNthFromEnd(struct ListNode *head, int n)
{
	struct ListNode *end = head;
	struct ListNode *prev = head;
	for (int i = 0; i < n; ++i)
		end = end->next;
	// It is not said whether need to free memory. Hence it is not freed.
	if (end == NULL)
		return head->next;
	end = end->next;
	if (end == NULL) {
		head->next = head->next->next;
		return head;
	}
	do {
		end = end->next;
		prev = prev->next;
	} while (end != NULL);
	prev->next = prev->next->next;
	return head;
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static inline void
check_res(int *nums, int count, int n, const int *expected, int expected_count)
{
	struct ListNode *nodes = malloc(count * sizeof(nodes[0]));
	for (int i = 0; i < count; ++i) {
		nodes[i].val = nums[i];
		if (i > 0)
			nodes[i - 1].next = &nodes[i];
	}
	nodes[count - 1].next = NULL;

	struct ListNode *res = removeNthFromEnd(nodes, n);
	if (expected_count == 0) {
		assert(res == NULL);
	} else {
		int i = 0;
		while (res != NULL) {
			check(i < expected_count);
			check(expected[i] == res->val);
			++i;
			res = res->next;
		}
		check(i == expected_count);
	}
	free(nodes);
}

int
main()
{
	{
		int nums[] = {1, 2, 3, 4, 5};
		int expected[] = {1, 2, 3, 5};

		check_res(nums, 5, 2, expected, 4);
	}
	{
		int nums[] = {1, 2, 3};
		int expected[] = {2, 3};

		check_res(nums, 3, 3, expected, 2);
	}
	{
		int nums[] = {1, 2, 3};
		int expected[] = {1, 2};

		check_res(nums, 3, 1, expected, 2);
	}
	{
		int nums[] = {1, 2};
		int expected[] = {1};

		check_res(nums, 2, 1, expected, 1);
	}
	{
		int nums[] = {1};
		check_res(nums, 1, 1, NULL, 0);
	}
	return 0;
}
