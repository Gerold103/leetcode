#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct ListNode {
	int val;
	struct ListNode *next;
};


struct ListNode*
addTwoNumbers(struct ListNode* l1, struct ListNode* l2)
{
	const int limit = 100;
	int digs[limit + 1];
	int count = 0;
	int carry = 0, d;
	while (l1 != NULL || l2 != NULL) {
		d = carry;
		if (l1 != NULL) {
			d += l1->val;
			l1 = l1->next;
		}
		if (l2 != NULL) {
			d += l2->val;
			l2 = l2->next;
		}
		if (d >= 10) {
			assert(d < 20);
			carry = 1;
			d -= 10;
		} else {
			carry = 0;
		}
		digs[count++] = d;
	}
	if (carry > 0)
		digs[count++] = carry;

	struct ListNode* nodes = malloc(sizeof(nodes[0]) * count);
	int last = count - 1;
	for (int i = 0; i < last; ++i) {
		nodes[i].val = digs[i];
		nodes[i].next = &nodes[i + 1];
	}
	nodes[last].val = digs[last];
	nodes[last].next = NULL;
	return nodes;
}

static void
consume_list(struct ListNode *n)
{
	struct ListNode *orig = n;
	printf("[%d", n->val);
	while ((n = n->next) != NULL)
		printf(", %d", n->val);
	printf("]\n");
	free(orig);
}

int
main()
{
	struct ListNode n11;
	struct ListNode n12;
	struct ListNode n13;
	struct ListNode n21;
	struct ListNode n22;
	struct ListNode n23;

	n11.next = NULL;
	n11.val = 1;
	n21.next = NULL;
	n21.val = 2;
	consume_list(addTwoNumbers(&n11, &n21));

	n11.val = 5;
	n21.val = 5;
	consume_list(addTwoNumbers(&n11, &n21));

	// 19
	n11.val = 9;
	n11.next = &n12;
	n12.val = 1;
	n12.next = NULL;
	// 122
	n21.val = 2;
	n21.next = &n22;
	n22.val = 2;
	n22.next = &n23;
	n23.val = 1;
	n23.next = NULL;
	consume_list(addTwoNumbers(&n11, &n21));

	// 99
	n12.val = 9;
	// 999
	n21.val = 9;
	n22.val = 9;
	n23.val = 9;
	consume_list(addTwoNumbers(&n11, &n21));
	return 0;
}
