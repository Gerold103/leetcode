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
swapPairs(struct ListNode *head)
{
	struct ListNode *n1 = head;
	if (n1 == NULL)
		return NULL;
	struct ListNode *n2 = n1->next;
	if (n2 == NULL)
		return head;
	head = n2;
	n1->next = n2->next;
	n2->next = n1;

	struct ListNode *prev = n1;
	while (true) {
		n1 = prev->next;
		if (n1 == NULL)
			break;
		n2 = n1->next;
		if (n2 == NULL)
			break;
		n1->next = n2->next;
		n2->next = n1;
		prev->next = n2;
		prev = n1;
	}
	return head;
}

static struct ListNode *
make_list(const int *values, int count)
{
	struct ListNode *nodes = malloc(sizeof(nodes[0]) * count);
	for (int i = 0; i < count; ++i) {
		nodes[i].val = values[i];
		if (i != 0)
			nodes[i - 1].next = &nodes[i];
	}
	nodes[count - 1].next = NULL;
	return nodes;
}

static bool
lists_are_eq(const struct ListNode *left, const struct ListNode *right)
{
	while (left != NULL && right != NULL) {
		if (left->val != right->val)
			return false;
		left = left->next;
		right = right->next;
	}
	return left == NULL && right == NULL;
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
		struct ListNode *src = make_list(
			(const int[]){1, 2, 3, 4, 5, 6}, 6);
		struct ListNode *expected = make_list(
			(const int[]){2, 1, 4, 3, 6, 5}, 6);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		struct ListNode *src = make_list(
			(const int[]){1, 2, 3, 4, 5}, 5);
		struct ListNode *expected = make_list(
			(const int[]){2, 1, 4, 3, 5}, 5);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		struct ListNode *src = make_list(
			(const int[]){1, 2, 3, 4}, 4);
		struct ListNode *expected = make_list(
			(const int[]){2, 1, 4, 3}, 4);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		struct ListNode *src = make_list(
			(const int[]){1, 2, 3}, 3);
		struct ListNode *expected = make_list(
			(const int[]){2, 1, 3}, 3);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		struct ListNode *src = make_list(
			(const int[]){1, 2}, 2);
		struct ListNode *expected = make_list(
			(const int[]){2, 1}, 2);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		struct ListNode *src = make_list(
			(const int[]){1}, 1);
		struct ListNode *expected = make_list(
			(const int[]){1}, 1);
		struct ListNode *actual = swapPairs(src);
		check(lists_are_eq(actual, expected));
		free(src);
		free(expected);
	}
	{
		check(swapPairs(NULL) == NULL);
	}
	return 0;
}
