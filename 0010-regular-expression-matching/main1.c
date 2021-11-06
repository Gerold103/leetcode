#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

// Simple solution, optimal in terms of memory usage.
static bool
isMatchBruteForce(const char *s, const char *p)
{
next:
	if (*p == 0)
		return *s == 0;
	if (*s == *p || *p == '.') {
		++p;
		if (*p != '*') {
			// Regular symbol or '.'. Match them one by one.
			if (*s == 0)
				return false;
			++s;
			goto next;
		}
		++p;
		// Try to skip '*' like it is empty.
		if (isMatchBruteForce(s, p))
			return true;
		if (*s == 0)
			return false;
		// Try to skip 1 symbol from the source str. Skip of multiple
		// strings happens when this place is called in recursion
		// on each next call.
		return isMatchBruteForce(s + 1, p - 2);
	}
	// Even if didn't match, it could be that the current symbol of the
	// patter is actually 'x*' - then just skip it.
	if (*p == 0)
		return false;
	++p;
	if (*p != '*')
		return false;
	++p;
	goto next;
}

// Dynamic programming recursive solution. It is almost the same as the brute
// force one, but the results of previous iterations are reused when possible.
// For each (i, j) store a sign whether match(s[i:], p[j:]) is true/false/
// unknown. When non-unknown - no need to redo the work.
static bool
isMatchDynamicRecurse(int8_t** dp, const char *sstart, const char *s,
		      const char *pstart, const char *p)
{
	// Check if already visited this combination.
	if (dp[s - sstart][p - pstart] != 0)
		return dp[s - sstart][p - pstart] == 1;
next:
	// The algorithm is the same as for brute force, but in the end remember
	// the result into the cache.
	if (*p == 0) {
		if (*s == 0)
			goto return_true;
		goto return_false;
	}
	if (*s == *p || *p == '.') {
		++p;
		if (*p != '*') {
			if (*s == 0)
				goto return_false;
			++s;
			goto next;
		}
		++p;
		if (isMatchDynamicRecurse(dp, sstart, s, pstart, p))
			goto return_true;
		if (*s == 0)
			goto return_false;
		++s;
		if (isMatchDynamicRecurse(dp, sstart, s, pstart, p - 2))
			goto return_true;
		goto return_false;
	}
	if (*p == 0)
		goto return_false;
	++p;
	if (*p != '*')
		goto return_false;
	++p;
	goto next;

return_true:
	dp[s - sstart][p - pstart] = 1;
	return true;

return_false:
	dp[s - sstart][p - pstart] = -1;
	return false;
}

static bool
isMatchDynamic(const char *str, const char *pat)
{
	// Task condition gives the restrictions.
	const int row_count = 21;
	const int col_count = 31;
	int8_t data[row_count][col_count];
	memset(data, 0, sizeof(data));
	int8_t *rows[row_count];
	for (int i = 0; i < row_count; ++i)
		rows[i] = data[i];
	return isMatchDynamicRecurse(rows, str, str, pat, pat);
}

static void
check(bool ok)
{
	if (!ok)
		abort();
}

static void
test(bool (*func)(const char *, const char *))
{
	check(!func("aaaaaaaaaaaaab", "a*a*a*a*a*a*a*a*a*a*c"));
	check(func("a", "a"));
	check(!func("a", ".*..a*"));
	check(!func("ab", ".*c"));
	check(func("aab", "c*a*b"));
	check(func("aa", "aab*c*d*e*"));
	check(!func("aa", "a"));
	check(func("aa", "a*"));
	check(func("ab", ".*"));
	check(func("abqewvd", "ab.*w.d*"));
	check(!func("mississippi", "mis*is*p*."));
}

int
main()
{
	test(isMatchDynamic);
	test(isMatchBruteForce);
	return 0;
}
