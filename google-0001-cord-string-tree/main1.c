#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

struct node {
	// When not-NULL - it is a leaf.
	const char *value;
	// For leaf - length of the value. For internal nodes - length of the
	// substring in this subtree.
	int len;
	// Both optional.
	struct node *left;
	struct node *right;
};

// Get character in the given position in the string stored as a cord.
static char
cord_get_char(const struct node *root, int idx)
{
	if (idx >= root->len || idx < 0)
		return 0;
	const struct node *n = root;
	while (n->value == NULL) {
		if (n->left == NULL) {
			n = n->right;
		} else if (n->left->len > idx) {
			n = n->left;
		} else {
			idx -= n->left->len;
			n = n->right;
		}
	}
	return n->value[idx];
}

// Recursive helper for getting a substring. Node is a subtree. Out is where to
// write result. Idx is where in the subtree need to start. Len is how much need
// to copy.
// Returns how many was actually copied. Might be less then len. Then the caller
// need to walk more subtrees.
static int
cord_get_substr_recursive(const struct node *node, char *out, int idx, int len)
{
	// There might be empty leafs not having anything.
	if (node == NULL)
		return 0;
	if (node->value != NULL) {
		assert(idx < node->len);
		int copy_len = len;
		int avail = node->len - idx;
		if (copy_len > avail)
			copy_len = avail;
		memcpy(out, node->value + idx, copy_len);
		return copy_len;
	}
	// If only one child, then the walking is straightforward.
	if (node->right == NULL)
		return cord_get_substr_recursive(node->left, out, idx, len);
	if (node->left == NULL)
		return cord_get_substr_recursive(node->right, out, idx, len);

	// Filter out the left child.
	if (node->left->len <= idx) {
		idx -= node->left->len;
		return cord_get_substr_recursive(node->right, out, idx, len);
	}

	// The core part of the algorithm - jump from left to right subtree.
	int used = cord_get_substr_recursive(node->left, out, idx, len);
	assert(used <= len);
	len -= used;
	if (len == 0)
		return used;
	out += used;
	return used + cord_get_substr_recursive(node->right, out, 0, len);
}

static char *
cord_get_substr(const struct node *root, int idx, int len)
{
	if (idx + len > root->len || len <= 0 || idx < 0)
		return NULL;
	char *res = malloc(len + 1);
	res[len] = 0;
	cord_get_substr_recursive(root, res, idx, len);
	return res;
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static void
test_on_tree(const struct node *root, const char *str)
{
	int len = strlen(str);
	// Get-char tests.
	for (int i = 0; i < len; ++i)
		check(cord_get_char(root, i) == str[i]);
	check(cord_get_char(root, -1) == 0);
	check(cord_get_char(root, len) == 0);
	check(cord_get_char(root, len + 1) == 0);

	// Get-substr tests.
	for (int i = -1; i <= len + 1; ++i) {
		for (int l = -1; l <= len + 1; ++l) {
			char *res = cord_get_substr(root, i, l);
			if (l <= 0 || i < 0 || i + l > len) {
				check(res == NULL);
				continue;
			}
			check(strlen(res) == l);
			check(strncmp(res, str + i, l) == 0);
			free(res);
		}
	}
}

int
main(void)
{
	// Simple full tree.
	{
		//            *
		//         /    \
		//        *      *
		//       / \    / \
		//    abc  de fghk lmn
		struct node nodes[7] = {0};
		nodes[0].left = &nodes[1];
		nodes[0].right = &nodes[2];
		nodes[0].len = 12;
		nodes[1].left = &nodes[3];
		nodes[1].right = &nodes[4];
		nodes[1].len = 5;
		nodes[2].left = &nodes[5];
		nodes[2].right = &nodes[6];
		nodes[2].len = 7;
		nodes[3].value = "abc";
		nodes[3].len = 3;
		nodes[4].value = "de";
		nodes[4].len = 2;
		nodes[5].value = "fghk";
		nodes[5].len = 4;
		nodes[6].value = "lmn";
		nodes[6].len = 3;

		test_on_tree(nodes, "abcdefghklmn");
	}
	// A bit fractured tree.
	{
		//             0
		//           /   \
		//         /      \
		//        1        8
		//         \      / \
		//         2     9   10
		//        /     fg    \
		//        3            11
		//      /   \        /    \
		//      4   7      12      13
		//    /  \  de    hklm    / \
		//   5   6               14  15
		//  abc  -               n  / \
		//                        16  17
		//                         o   p
		struct node nodes[18] = {0};
		// Build it top to bottom, left to right.
		nodes[0].left = &nodes[1];
		nodes[0].right = &nodes[8];
		nodes[0].len = 14;

		nodes[1].right = &nodes[2];
		nodes[1].len = 5;

		nodes[2].left = &nodes[3];
		nodes[2].len = 5;

		nodes[3].left = &nodes[4];
		nodes[3].right = &nodes[7];
		nodes[3].len = 5;

		nodes[4].left = &nodes[5];
		nodes[4].right = &nodes[6];
		nodes[4].len = 3;

		nodes[5].value = "abc";
		nodes[5].len = 3;

		nodes[7].value = "de";
		nodes[7].len = 2;

		nodes[8].left = &nodes[9];
		nodes[8].right = &nodes[10];
		nodes[8].len = 8;

		nodes[9].value = "fg";
		nodes[9].len = 2;

		nodes[10].right = &nodes[11];
		nodes[10].len = 6;

		nodes[11].left = &nodes[12];
		nodes[11].right = &nodes[13];
		nodes[11].len = 6;

		nodes[12].value = "hklm";
		nodes[12].len = 4;

		nodes[13].left = &nodes[14];
		nodes[13].right = &nodes[15];
		nodes[13].len = 3;

		nodes[14].value = "n";
		nodes[14].len = 1;

		nodes[15].left = &nodes[16];
		nodes[15].right = &nodes[17];
		nodes[15].len = 2;

		nodes[16].value = "o";
		nodes[16].len = 1;

		nodes[17].value = "p";
		nodes[17].len = 1;

		test_on_tree(nodes, "abcdefghklmnop");
	}
	return 0;
}
