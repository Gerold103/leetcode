#pragma once

#include <cstdio>
#include <cstdlib>

#define unit_test_start() \
	printf("\t-------- %s started --------\n", __func__)

#define unit_test_finish() \
	printf("\t-------- %s done --------\n", __func__)

#define unit_assert(cond) do {						\
	if (!(cond)) {							\
		printf("Test failed, line %d\n", __LINE__);		\
		exit(-1);						\
	}								\
} while (0)

#define unit_msg(...) do {						\
	printf("# ");							\
	printf(__VA_ARGS__);						\
	printf("\n");							\
} while (0)

#define unit_check(cond, msg) do {					\
	if (! (cond)) {							\
		printf("not ok - %s\n", (msg));				\
		unit_assert(false);					\
	} else {							\
		printf("ok - %s\n", (msg));				\
	}								\
} while(0)
