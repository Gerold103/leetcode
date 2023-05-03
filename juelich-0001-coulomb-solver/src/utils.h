#pragma once

#include <cstdint>

enum {
	COULOMB_CACHE_LINE_SIZE = 64,
	COULOMB_MAX_THREAD_COUNT = 20,
	COULOMB_MAX_PARTICLE_COUNT = 1000000,
};

// Compare 2 double numbers taking into account that they might be not bitwise
// equal due to different rounding errors. But still should be considered equal
// with certain precision.
static inline bool
double_is_eq(
	double a,
	double b)
{
	constexpr double float_epsilon = 0.0000001;
	return a + float_epsilon >= b && a - float_epsilon <= b;
}

// Monotonic timestamp in nanoseconds.
uint64_t
time_monotonic_nanosec();

// Convert the string to a number. The string must not have any symbols except
// spaces and the single number.
bool
string_to_num(
	const char* str,
	unsigned int& out);

// Read a number from the string. The string position is updated on success.
// Can have leading spaces. After the number must either be more spaces or end
// of the string.
bool
string_read_num(
	const char*& str,
	double& out);
