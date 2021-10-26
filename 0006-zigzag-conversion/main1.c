#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

char *
convert(const char *s, int row_count)
{
	assert(row_count >= 1);
	if (row_count == 1)
		return strdup(s);

	int len = strlen(s);
	char *res = malloc(len + 1);
	res[len] = 0;
	char *out = res;

	/* First row. */
	int row_count_1 = row_count - 1;
	int steps[2] = {row_count_1 * 2, 0};
	for (int j = 0; j < len; j += steps[0])
		*(out++) = s[j];
	steps[0] -= 2;
	steps[1] += 2;

	/* Internal rows. */
	for (int i = 1; i < row_count_1; ++i) {
		int step_i = 0;
		for (int j = i; j < len; j += steps[step_i], step_i = !step_i)
			*(out++) = s[j];
		steps[0] -= 2;
		steps[1] += 2;
	}

	/* Last row. */
	for (int j = row_count_1; j < len; j += steps[1])
		*(out++) = s[j];
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
	consumeStr(convert("abcdefghijklmnoprst", 5));
	consumeStr(convert("abcdefghijklmnoprst", 4));
	consumeStr(convert("abcdefghijklmnoprst", 3));
	consumeStr(convert("abcdefghijklmnoprst", 2));
	consumeStr(convert("abcdefghijklmnoprst", 1));
	consumeStr(convert("PAYPALISHIRING", 3));
	consumeStr(convert("PAYPALISHIRING", 4));
	consumeStr(convert("A", 1));
	return 0;
}
