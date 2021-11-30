#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

char *
longestCommonPrefix(const char *const* strs, int count)
{
	if (count == 0)
		return strdup("");
	int size = 0;
	char c;
	while ((c = strs[0][size]) != 0) {
		for (int i = 1; i < count; ++i) {
			if (c != strs[i][size])
				goto finish;
		}
		++size;
	}
finish:
	return strndup(strs[0], size);
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static inline void
consume_res(char *got, const char *expected)
{
	check(strcmp(got, expected) == 0);
	free(got);
}

int
main()
{
	{
		const char *strs[] = {"flower", "flow", "flight"};
		consume_res(longestCommonPrefix(strs, 3), "fl");
	}
	{
		const char *strs[] = {"dog","racecar","car"};
		consume_res(longestCommonPrefix(strs, 3), "");
	}
	return 0;
}
