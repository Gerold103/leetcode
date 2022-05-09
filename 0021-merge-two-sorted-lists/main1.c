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
mergeTwoLists(struct ListNode *list1, struct ListNode *list2)
{
	if (list1 == NULL)
		return list2;
	if (list2 == NULL)
		return list1;
	struct ListNode *first;
	struct ListNode *res;
	if (list1->val <= list2->val) {
		res = list1;
		list1 = list1->next;
	} else {
		res = list2;
		list2 = list2->next;
	}
	first = res;
	while (list1 != NULL && list2 != NULL) {
		if (list1->val <= list2->val) {
			res->next = list1;
			res = list1;
			list1 = list1->next;
		} else {
			res->next = list2;
			res = list2;
			list2 = list2->next;
		}
	}
	if (list1 != NULL)
		res->next = list1;
	else
		res->next = list2;
	return first;
}

static struct ListNode *
make_list(const int *values, int count)
{
	if (count == 0)
		return NULL;
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
		struct ListNode *list1 = make_list(
			(const int[]){1, 2, 4}, 3);
		struct ListNode *list2 = make_list(
			(const int[]){1, 3, 4}, 3);
		struct ListNode *expected = make_list(
			(const int[]){1, 1, 2, 3, 4, 4}, 6);
		struct ListNode *actual = mergeTwoLists(list1, list2);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(list1);
		free(list2);
	}
	{
		struct ListNode *list1 = make_list((const int[]){}, 0);
		struct ListNode *list2 = make_list((const int[]){}, 0);
		struct ListNode *expected = make_list((const int[]){}, 0);
		struct ListNode *actual = mergeTwoLists(list1, list2);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(list1);
		free(list2);
	}
	{
		struct ListNode *list1 = make_list((const int[]){}, 0);
		struct ListNode *list2 = make_list((const int[]){0}, 1);
		struct ListNode *expected = make_list((const int[]){0}, 1);
		struct ListNode *actual = mergeTwoLists(list1, list2);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(list1);
		free(list2);
	}
	{
		struct ListNode *list1 = make_list(
			(const int[]){3, 5}, 2);
		struct ListNode *list2 = make_list(
			(const int[]){1, 4, 6, 7}, 4);
		struct ListNode *expected = make_list(
			(const int[]){1, 3, 4, 5, 6, 7}, 6);
		struct ListNode *actual = mergeTwoLists(list1, list2);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(list1);
		free(list2);
	}
	{
		struct ListNode *list1 = make_list(
			(const int[]){3, 6, 7, 10}, 4);
		struct ListNode *list2 = make_list(
			(const int[]){-1, 2, 15}, 3);
		struct ListNode *expected = make_list(
			(const int[]){-1, 2, 3, 6, 7, 10, 15}, 7);
		struct ListNode *actual = mergeTwoLists(list1, list2);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(list1);
		free(list2);
	}
	return 0;
}
