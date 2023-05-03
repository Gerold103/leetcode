#include "utils.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <type_traits>

#include "stdio.h"

template<typename T>
static bool
string_read_num_uint(
	const char*& str,
	T& out)
{
	static_assert(!std::is_signed_v<T>, "only unsigned is supported");
	while (isspace(*str))
		++str;
	// Check for digit explicitly. Otherwise strtoull() takes '-' sign and
	// returns garbage - unsigned can't be negative.
	if (!isdigit(*str))
		return false;
	char *end = nullptr;
	errno = 0;
	unsigned long long res = strtoull(str, &end, 10);
	if (errno != 0 || *str == 0)
		return false;
	if (res > std::numeric_limits<T>::max())
		return false;
	str = end;
	out = (T)res;
	return true;
}

static bool
string_read_num_double(
	const char*& str,
	double& out)
{
	while (isspace(*str))
		++str;
	char *end = nullptr;
	errno = 0;
	out = strtod(str, &end);
	if (errno != 0 || *str == 0)
		return false;
	if (*end != 0 && !isspace(*end))
		return false;
	str = end;
	return true;
}

uint64_t
time_monotonic_nanosec()
{
	// std::chrono has some time functions, but 1) 'steady' clock doesn't give
	// any precision guarantees, 2) 'high res' and 'system' clocks might be not
	// monotonic. Have to use the low-level API.
	timespec spec;
	int rc = clock_gettime(CLOCK_MONOTONIC, &spec);
	assert(rc == 0);
	(void)rc;
	return spec.tv_sec * 1000000000 + spec.tv_nsec;
}

bool
string_to_num(
	const char* str,
	unsigned int& out)
{
	if (!string_read_num_uint(str, out))
		return false;
	while (isspace(*str))
		++str;
	return *str == 0;
}

bool
string_read_num(
	const char*& str,
	double& out)
{
	return string_read_num_double(str, out);
}
