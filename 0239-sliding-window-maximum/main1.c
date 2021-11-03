#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

struct queue {
	int *data;
	int cap;
	int begin;
	int end;
};

static void
queue_create(struct queue *q, int cap)
{
	q->data = malloc(cap * sizeof(*q->data));
	q->cap = cap;
	q->begin = 0;
	q->end = 0;
}

static void
queue_destroy(struct queue *q)
{
	free(q->data);
}

static void
queue_add_last(struct queue *q, int val)
{
	assert(q->end - q->begin < q->cap);
	q->data[q->end++ % q->cap] = val;
}

static void
queue_pop_last(struct queue *q)
{
	assert(q->end > q->begin);
	--q->end;
}

static void
queue_pop_first(struct queue *q)
{
	assert(q->end > q->begin);
	++q->begin;
}

static int
queue_last(const struct queue *q)
{
	assert(q->end > q->begin);
	return q->data[(q->end - 1) % q->cap];
}

static int
queue_first(const struct queue *q)
{
	assert(q->end > q->begin);
	return q->data[q->begin % q->cap];
}

static int
queue_count(const struct queue *q)
{
	return q->end - q->begin;
}

static int
queue_is_empty(const struct queue *q)
{
	return q->end == q->begin;
}

int *
maxSlidingWindow(int *nums, int num_count, int k, int *res_count)
{
	// The core idea is that don't need to store all the elements of the
	// window. In a window can drop the elements which are not going to be
	// maximal in any window. These are values who have a bigger value
	// following them in the window. For instance, if a value A <= B and A
	// is earlier in the window than B, then storing A makes no sense. It
	// won't be maximal in one window with B ever, and will fall out of the
	// window earlier than B.
	assert(k <= num_count);
	// The queue stores indexes, not the elements themselves, because
	// otherwise can't tell when the first element should be popped.
	// With the algorithm above it works that nums[queue_first()] is the
	// maximal element of the window - it purged all the previous items.
	struct queue q;
	queue_create(&q, k + 1);

	int res_i = 0;
	*res_count = num_count - k + 1;
	int *res = malloc(*res_count * sizeof(*res));

	// Populate the first part of the window without trying to pop firsts.
	// Window only grows here anyway.
	for (int i = 0; i < k - 1; ++i) {
		while (!queue_is_empty(&q) && nums[queue_last(&q)] <= nums[i])
			queue_pop_last(&q);
		queue_add_last(&q, i);
	}
	// Now the heavy part with more 'if's when the first items can fall out
	// of the border.
	for (int i = k - 1; i < num_count; ++i) {
		while (!queue_is_empty(&q) && nums[queue_last(&q)] <= nums[i])
			queue_pop_last(&q);
		queue_add_last(&q, i);
		int first = queue_first(&q);
		res[res_i] = nums[first];
		if (first == res_i)
			queue_pop_first(&q);
		++res_i;
	}

	queue_destroy(&q);
	return res;
}

static void
consume_res(int *res, int count)
{
	printf("[");
	for (int i = 0; i < count; ++i) {
		if (i != 0)
			printf(", ");
		printf("%d", res[i]);
	}
	printf("]\n");
	free(res);
}

int
main()
{
	int count;
	int *res;
	int res_count;
	{
		int nums[] = {
			8, 7, 6, 5, 4, 3, 2, 1, 0
		};
		count = sizeof(nums) / sizeof(nums[0]);

		res = maxSlidingWindow(nums, count, 1, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 2, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 3, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 4, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 5, &res_count);
		consume_res(res, res_count);
	}
	{
		int nums[] = {
			9, 5, 6, 6, 2, 3, 2, 4, 3, 1, 2, 3
		};
		count = sizeof(nums) / sizeof(nums[0]);

		res = maxSlidingWindow(nums, count, 3, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 4, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 5, &res_count);
		consume_res(res, res_count);
	}
	{
		int nums[] = {
			1, 3, -1, -3, 5, 3, 6, 7
		};
		count = sizeof(nums) / sizeof(nums[0]);

		res = maxSlidingWindow(nums, count, 3, &res_count);
		consume_res(res, res_count);
	}
	{
		int num = 1;
		res = maxSlidingWindow(&num, 1, 1, &res_count);
		consume_res(res, res_count);
	}{
		int nums[] = {
			1, -1
		};
		count = sizeof(nums) / sizeof(nums[0]);

		res = maxSlidingWindow(nums, count, 1, &res_count);
		consume_res(res, res_count);

		res = maxSlidingWindow(nums, count, 2, &res_count);
		consume_res(res, res_count);
	}
	return 0;
}
