#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

static const char *pads[] = {
	[2] = "abc",
	[3] = "def",
	[4] = "ghi",
	[5] = "jkl",
	[6] = "mno",
	[7] = "pqrs",
	[8] = "tuv",
	[9] = "wxyz",
};

char **
letterCombinations(const char *digits, int *out_count)
{
	if (*digits == 0) {
		*out_count = 0;
		return NULL;
	}
	int count = 1;
	const char *dpos = digits;
	do {
		count *= strlen(pads[*dpos - '0']);
		++dpos;
	} while (*dpos != 0);
	int len = dpos - digits;
	*out_count = count;

	char **res = (char **)malloc((len + 1 + sizeof(char *)) * count);
	char *pos = (char *)res + count * sizeof(char *);
	for (int i = 0; i < count; ++i) {
		res[i] = pos;
		res[i][len] = 0;
		pos += len + 1;
	}

	int dupcount = 1;
	for (int di = 0; di < len; ++di) {
		const char *pad = pads[digits[di] - '0'];
		int padlen = strlen(pad);
		int resi = 0;
		while (resi < count) {
			for (int padi = 0; padi < padlen; ++padi) {
				for (int dupi = 0; dupi < dupcount;
				     ++dupi, ++resi)
					res[resi][di] = pad[padi];
			}
		}
		dupcount *= padlen;
	}
	return res;
}

static inline void
check(const char *digits, const char **res, int res_count)
{
	int count = 0;
	char **ret = letterCombinations(digits, &count);
	if (res_count != count)
		abort();
	for (int i = 0; i < count; ++i) {
		if (strcmp(ret[i], res[i]) != 0)
			abort();
	}
}

int
main()
{
	{
		const char *res[9] = {
			"ad", "bd", "cd", "ae", "be", "ce", "af", "bf", "cf"
		};
		check("23", res, 9);
	}
	{
		const char *res[36] = {
			"dmw", "emw", "fmw", "dnw", "enw", "fnw", "dow", "eow",
			"fow", "dmx", "emx", "fmx", "dnx", "enx", "fnx", "dox",
			"eox", "fox", "dmy", "emy", "fmy", "dny", "eny", "fny",
			"doy", "eoy", "foy", "dmz", "emz", "fmz", "dnz", "enz",
			"fnz", "doz", "eoz", "foz",
		};
		check("369", res, 36);
	}
	{
		check("", NULL, 0);
	}
	return 0;
}
