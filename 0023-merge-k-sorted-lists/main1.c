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

static void
heap_push(struct ListNode **heap, int *size, struct ListNode *item)
{
	int idx = (*size)++;
	heap[idx] = item;
	int parent_idx;
	while ((parent_idx = (idx - 1) / 2) >= 0) {
		struct ListNode *parent = heap[parent_idx];
		if (parent->val <= item->val)
			break;
		heap[parent_idx] = item;
		heap[idx] = parent;
		idx = parent_idx;
	}
}

static struct ListNode *
heap_pop(struct ListNode **heap, int *size)
{
	if (*size == 0)
		return NULL;
	struct ListNode *res = heap[0];
	int new_size = --*size;
	heap[0] = heap[new_size];
	int idx = 0;
	int left_idx, right_idx, min_idx;
	struct ListNode *left, *right, *item, *min_item;
	while (true) {
		left_idx = idx * 2 + 1;
		right_idx = left_idx + 1;
		if (left_idx < new_size) {
			left = heap[left_idx];
			if (right_idx < new_size) {
				right = heap[right_idx];
				if (left->val <= right->val) {
					min_idx = left_idx;
					min_item = left;
				} else {
					min_idx = right_idx;
					min_item = right;
				}
			} else {
				min_idx = left_idx;
				min_item = left;
			}
		} else if (right_idx < new_size) {
			min_idx = right_idx;
			min_item = right;
		} else {
			break;
		}
		item = heap[idx];
		if (item->val <= min_item->val)
			break;
		heap[min_idx] = item;
		heap[idx] = min_item;
		idx = min_idx;
	}
	return res;
}

struct ListNode*
mergeKLists(struct ListNode **lists, int count)
{
	const int static_heap_cap = 10;
	struct ListNode *static_heap[static_heap_cap];

	struct ListNode **heap;
	int heap_size = 0;
	if (count <= static_heap_cap)
		heap = static_heap;
	else
		heap = malloc(sizeof(heap[0]) * count);

	for (int i = 0; i < count; ++i) {
		struct ListNode *list = lists[i];
		if (list != NULL)
			heap_push(heap, &heap_size, list);
	}

	struct ListNode *first = heap_pop(heap, &heap_size);
	if (first == NULL)
		goto end;

	struct ListNode *last = first;
	if (first->next != NULL)
		heap_push(heap, &heap_size, first->next);
	struct ListNode *next;
	while ((next = heap_pop(heap, &heap_size)) != NULL) {
		last->next = next;
		last = next;
		if (next->next != NULL)
			heap_push(heap, &heap_size, next->next);
	}
	assert(last->next == NULL);
end:
	if (heap != static_heap)
		free(heap);
	return first;
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
		struct ListNode *lists[] = {
			make_list((const int[]){1, 4, 5}, 3),
			make_list((const int[]){1, 3, 4}, 3),
			make_list((const int[]){2, 6}, 2),
		};
		struct ListNode *expected = make_list(
			(const int[]){1, 1, 2, 3, 4, 4, 5, 6}, 8);
		struct ListNode *actual = mergeKLists(lists, 3);
		check(lists_are_eq(actual, expected));
		free(expected);
		free(lists[0]);
		free(lists[1]);
		free(lists[2]);
	}
	{
		struct ListNode *lists[] = {
			make_list((const int[]){-10, -9, -9, -3, -1, -1, 0}, 7),
			make_list((const int[]){-5}, 1),
			make_list((const int[]){4}, 1),
			make_list((const int[]){-8}, 1),
			NULL,
			make_list((const int[]){-9, -6, -5, -4, -2, 2, 3}, 7),
			make_list((const int[]){-3, -3, -2, -1, 0}, 5),
		};
		struct ListNode *expected = make_list(
			(const int[]){-10, -9, -9, -9, -8, -6, -5, -5, -4, -3,
				-3, -3, -2, -2, -1, -1, -1, 0, 0, 2, 3, 4}, 22);
		struct ListNode *actual = mergeKLists(lists, 7);
		check(lists_are_eq(actual, expected));
		free(expected);
		for (int i = 0; i < 7; ++i)
			free(lists[i]);
	}
	return 0;
}
