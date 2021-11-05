#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

int
myAtoi(const char *s)
{
	int sign = 1;
	int64_t res = 0;
	char c = *s;
	// The task condition says only ' ' should be considered a whitespace.
	// Hence not isspace().
	while (c == ' ')
		c = *++s;
	if (c == '-') {
		sign = -1;
		c = *++s;
	} else if (c == '+') {
		c = *++s;
	}
	if (c == 0)
		return 0;

	while (c <= '9' && c >= '0') {
		res = res * 10 + c - '0';
		if (res >= INT_MAX || c == 0)
			break;
		c = *++s;
	}

	res = res * sign;
	if (res >= INT_MAX)
		return INT_MAX;
	if (res <= INT_MIN)
		return INT_MIN;
	return (int)res;
}

int
main()
{
	printf("%d\n", myAtoi("42"));
	printf("%d\n", myAtoi("+1"));
	printf("%d\n", myAtoi("   -42"));
	printf("%d\n", myAtoi("4193 with words"));
	printf("%d\n", myAtoi("words and 987"));
	printf("%d\n", myAtoi("-91283472332"));
	return 0;
}
