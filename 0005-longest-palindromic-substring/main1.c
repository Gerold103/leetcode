#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static const char *
getPalindrome(const char *s, const char *pos, int *len)
{
	assert(*pos != 0);
	/* Check if this is a center of a palindrome. */
	const char *l = pos - 1, *r = pos + 1;
	const char *begin1 = pos;
	int len1 = 1;
	while (l >= s) {
		if (*r == 0)
			break;
		if (*l != *r)
			break;
		len1 += 2;
		begin1 = l;
		--l;
		++r;
	}
	/*
	 * Check if this is a first half of two-symbol center of a palindrome.
	 */
	const char *begin2 = pos;
	int len2 = 0;
	r = pos + 1;
	l = pos;
	while (l >= s) {
		if (*r == 0)
			break;
		if (*l != *r)
			break;
		len2 += 2;
		begin2 = l;
		--l;
		++r;
	}
	if (len2 > len1) {
		*len = len2;
		return begin2;
	}
	*len = len1;
	return begin1;
}

char *
longestPalindrome(const char* s)
{
	int max_len = 0, new_len;
	const char *max_begin = NULL, *new_begin;
	const char *pos = s;
	while (*pos != 0) {
		new_begin = getPalindrome(s, pos, &new_len);
		if (new_len > max_len) {
			max_len = new_len;
			max_begin = new_begin;
		}
		++pos;
	}
	char *res = malloc(max_len + 1);
	memcpy(res, max_begin, max_len);
	res[max_len] = 0;
	return res;
}

static void
consumeStr(char *str)
{
	printf("%s\n", str);
	free(str);
}

int
main()
{
	consumeStr(longestPalindrome("babad"));
	consumeStr(longestPalindrome("cbbd"));
	consumeStr(longestPalindrome("a"));
	consumeStr(longestPalindrome("ac"));
	return 0;
}
