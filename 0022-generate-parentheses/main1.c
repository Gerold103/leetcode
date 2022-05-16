#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

struct storage {
	int size;
	int capacity;
	int row_size;
	char** rows;
};

static inline void
storage_create(struct storage *s, int n)
{
	s->size = 0;
	s->capacity = 128;
	s->row_size = n * 2;
	s->rows = malloc(s->capacity * sizeof(s->rows[0]));
}

static inline void
storage_append(struct storage *s, const char *row)
{
	if (s->size == s->capacity) {
		s->capacity *= 2;
		s->rows = realloc(s->rows, s->capacity * sizeof(s->rows[0]));
	}
	char *copy = malloc(s->row_size + 1);
	memcpy(copy, row, s->row_size);
	copy[s->row_size] = 0;
	s->rows[s->size++] = copy;
}

static void
storage_fill(struct storage *s, char *row, int len, int max_n, int opened,
	int closed)
{
	if (opened < max_n) {
		row[len] = '(';
		storage_fill(s, row, len + 1, max_n, opened + 1, closed);
	} else if (closed == opened) {
		storage_append(s, row);
	}
	if (closed < opened) {
		row[len] = ')';
		storage_fill(s, row, len + 1, max_n, opened, closed + 1);
	}
}

static char **
generateParenthesis(int n, int *out_count)
{
	// Condition of the task.
	const int max_n = 8;
	assert(n <= max_n);
	char row[max_n * 2 + 1];
	struct storage s;
	storage_create(&s, n);
	storage_fill(&s, row, 0, n, 0, 0);
	*out_count = s.size;
	return s.rows;
}

static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

static inline void
check_rows_and_free(int count, char **rows, int expected_count,
	const char *expected_rows[])
{
	check(count == expected_count);
	for (int i = 0; i < count; ++i) {
		check(strcmp(rows[i], expected_rows[i]) == 0);
		free(rows[i]);
	}
	free(rows);
}

int
main()
{
	{
		int count = 0;
		char **res = generateParenthesis(1, &count);
		check_rows_and_free(count, res, 1, (const char *[]){"()"});
	}
	{
		int count = 0;
		char **res = generateParenthesis(3, &count);
		check_rows_and_free(count, res, 5, (const char *[]){
			"((()))","(()())","(())()","()(())","()()()"
		});
	}
	return 0;
}
