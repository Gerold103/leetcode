#include <stdlib.h>
#include <stdbool.h>

struct node {
	struct node *next;
	void *data;
};

static struct node *
revert_list(struct node *head)
{
	struct node *prev = NULL;
	while (head != NULL) {
		struct node *next = head->next;
		head->next = prev;
		prev = head;
		head = next;
	}
	return prev;
}

static void
check(bool ok)
{
	if (!ok)
		abort();
}

int
main()
{
	check(revert_list(NULL) == NULL);

	struct node node1 = {
		.next = NULL,
	};
	struct node *res = revert_list(&node1);
	check(res == &node1);
	check(res->next == NULL);

	struct node node2 = {
		.next = NULL,
	};
	node1.next = &node2;
	res = revert_list(&node1);
	check(res == &node2);
	check(node2.next == &node1);
	check(node1.next == NULL);

	struct node node3 = {
		.next = NULL,
	};
	node1.next = &node2;
	node2.next = &node3;
	res = revert_list(&node1);
	check(res == &node3);
	check(node3.next == &node2);
	check(node2.next == &node1);
	check(node1.next == NULL);

	struct node node4 = {
		.next = NULL,
	};
	node1.next = &node2;
	node2.next = &node3;
	node3.next = &node4;
	res = revert_list(&node1);
	check(res == &node4);
	check(node4.next == &node3);
	check(node3.next == &node2);
	check(node2.next == &node1);
	check(node1.next == NULL);
	return 0;
}
