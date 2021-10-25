#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int
lengthOfLongestSubstring(const char* s)
{
	uint8_t letters[256];
	memset(letters, 0, sizeof(letters));
	const char *begin = s;
	const char *end = s;
	int len, max_len = 0;
	while (*end != 0) {
		if (++letters[*end] != 1) {
			len = end - begin;
			if (len > max_len)
				max_len = len;
			while (--letters[*(begin++)] != 1);
		}
		++end;
	}
	len = end - begin;
	if (len > max_len)
		max_len = len;
	return max_len;
}

int
main()
{
	const char *str;

	str = "a";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	str = " ";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	str = "abcabcbb";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	str = "bbbbb";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	str = "pwwkew";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	str = "";
	printf("%s - %d\n", str, lengthOfLongestSubstring(str));

	return 0;
}
