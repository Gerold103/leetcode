There is a function sort(array, max_value, sort_factor) which should sort the
given array.

The items are integers. Each item is >= 0 and <= max_value. Array length can be
extremely huge.

Sort_factor is such a value that if array[N] == X, then for all i > N:
array[i] >= X - sort_factor.
