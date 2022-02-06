#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

bool
isValid(char *s)
{
	const int static_cap = 1024;
	char stack_static[static_cap];
	char *stack = stack_static;
	int stack_cap = static_cap;
	int stack_size = 0;
	bool rc = true;

	char c, opening;
	while ((c = *s) != 0) {
		switch(c) {
			case ')':
				opening = '(';
				goto close;
			case ']':
				opening = '[';
				goto close;
			case '}':
				opening = '{';
				goto close;
			default:
				goto open;
		}
	open:
		if (stack_size == stack_cap) {
			stack_cap *= 2;
			int byte_count = stack_cap * sizeof(stack[0]);
			if (stack != stack_static) {
				stack = realloc(stack, byte_count);
			} else {
				stack = malloc(byte_count);
				memcpy(stack, stack_static,
					stack_size * sizeof(stack[0]));
			}
		}
		stack[stack_size++] = c;
		goto next;

	close:
		if (stack_size == 0 || stack[stack_size - 1] != opening) {
			rc = false;
			break;
		}
		--stack_size;
		goto next;

	next:
		++s;
	}
	if (stack_size != 0)
		rc = false;
	if (stack != stack_static)
		free(stack);
	return rc;
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
	check(isValid("()"));
	check(isValid("()[]{}"));
	check(isValid("((([[]]{}{{}})){})"));
	check(!isValid("(]"));
	check(!isValid("("));
	check(!isValid("}"));
	check(!isValid("(([([[]]]{}{{}})){})"));
	return 0;
}
