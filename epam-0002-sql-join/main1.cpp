#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <thread>

// Doubles tend to loose precision, but for the task this value is enough.
static const double precision = 0.000001;
static const int limit = 10;

//////////////////////////////////////////////////
// Change these parameters to find the best combination for a given bench.

// Number of workers to use for the join stage.
static int worker_count = 5;
// Number of rows each worker takes for processing at once.
static int step_size = 100;
//////////////////////////////////////////////////

static inline bool
double_eq(double l, double r)
{
	double diff = l - r;
	return diff <= precision && diff >= -precision;
}

static inline bool
double_neq(double l, double r)
{
	double diff = l - r;
	return diff > precision || diff < -precision;
}

static inline bool
double_gt(double l, double r)
{
	return double_neq(l, r) && l > r;
}

static inline bool
double_ge(double l, double r)
{
	return double_eq(l, r) || l >= r;
}

static inline bool
double_lt(double l, double r)
{
	return double_neq(l, r) && l < r;
}

//////////////////////////////////////////////////

// Generic storage of a table's tuple. All rows in all tables are represented
// by it.
struct tuple {
	// Row id allows strict stable ordering for tuples whose other columns
	// match.
	int row_id;
	double col1;
	double col2;
};

// t1 < t2 in col1.
static bool
tuple_lt_col1(const tuple &t1, const tuple &t2)
{
	return double_lt(t1.col1, t2.col1);
}

// t1 > t2 in col2. But if equal, then order by row id.
static bool
tuple_st_gt_col2(const tuple &t1, const tuple &t2)
{
	if (double_neq(t1.col2, t2.col2))
		return double_gt(t1.col2, t2.col2);
	return t1.row_id < t2.row_id;
}

// t1 > t2 in col1.
static bool
tuple_gt_col1(const tuple &t1, const tuple &t2)
{
	return double_gt(t1.col1, t2.col1);
}

//////////////////////////////////////////////////

using table = std::vector<tuple>;

static void
table_read(table &tab, const char *path)
{
	tab.clear();
	std::ifstream file(path);
	int row_count;
	file >> row_count;
	tab.resize(row_count);
	int i = 0;
	for (tuple &t : tab) {
		t.row_id = i++;
		file >> t.col1 >> t.col2;
	}
}

static void
table_write_stream(const table &tab, std::ostream &stream)
{
	int size = tab.size();
	stream << size << '\n';
	stream << std::fixed << std::setprecision(6);
	for (const tuple &t : tab)
		stream << t.col1 << ' ' << t.col2 << '\n';
}

static void
table_write(const table &tab, const char *path)
{
	std::ofstream file(path);
	table_write_stream(tab, file);
}

static void
table_print(const table &tab)
{
	table_write_stream(tab, std::cout);
}

// Ascending sort by col1.
static void
table_sort_col1(table &t)
{
	std::sort(t.begin(), t.end(), tuple_lt_col1);
}

// Descending sort by col1.
static void
table_rsort_col1(table &t)
{
	std::sort(t.begin(), t.end(), tuple_gt_col1);
}

static const tuple *
table_upper_bound_lt_col1(const table &tab, const tuple &t)
{
	table::const_iterator pos = std::upper_bound(tab.cbegin(), tab.cend(),
		t, tuple_lt_col1);
	if (pos == tab.cend())
		return nullptr;
	return &*pos;
}

//////////////////////////////////////////////////

// Min stable tuple heap sorted by col2.
struct heap_ge_col2
{
	void
	reserve(int size)
	{ m_tab.reserve(size); }

	const tuple &
	top() const
	{ return m_tab.front(); }

	int
	size() const
	{ return m_tab.size(); }

	void
	pop();

	void
	push(const tuple &t);

	table m_tab;
};

inline void
heap_ge_col2::pop()
{
	std::pop_heap(m_tab.begin(), m_tab.end(), tuple_st_gt_col2);
	m_tab.pop_back();
}

inline void
heap_ge_col2::push(const tuple &t)
{
	m_tab.push_back(t);
	std::push_heap(m_tab.begin(), m_tab.end(), tuple_st_gt_col2);
}

//////////////////////////////////////////////////
// Cubical solution. But uses not much space.
//

// Microseconds diff between two timestamps.
static inline int64_t
timespec_diff_us(const struct timespec &l, const struct timespec &r)
{
	return (r.tv_sec - l.tv_sec) * 1000000 + (r.tv_nsec - l.tv_nsec) / 1000;
}

// Execute:
//
// SELECT t1.row_id, a, (SELECT SUM(x * y * z) FROM t2 JOIN t3 ON a < b + c);
//
static void
cubic_execute_sum(table &out, const table &t1, const table &t2, const table &t3)
{
	out.resize(t1.size());
	if (t2.empty() || t3.empty()) {
		int i = 0;
		for (const tuple &t : t1) {
			out[i] = {
				.row_id = t.row_id,
				.col1 = t.col1,
				.col2 = 0,
			};
		}
		return;
	}

	std::vector<std::thread> workers;
	workers.resize(worker_count);

	// The idea is to split the work between threads. It is relatively easy
	// to do, because all the 3 tables do not change in this query.
	//
	// Only need to sync the workers what ranges they take and where store
	// the results.
	//
	// The ranges are spread dynamically using an atomic counter. One by
	// one, first came - first served. That allows the threads evenly share
	// the load with the 'step_size' precision. This is vital, because
	// intervals can have totally different load inside. Some could be
	// skipped almost entirely, some would need true full scan.
	//
	std::atomic<int> t1_pos(0);
	for (int i = 0; i < worker_count; ++i) {
		workers[i] = std::thread([&]() {

	// Worker body start. ////////////////////

	int i1;
	int t1_size = t1.size();
	while ((i1 = t1_pos.fetch_add(step_size)) < t1_size) {
		int i2 = std::min(i1 + step_size, t1_size);
		for (int i = i1; i < i2; ++i) {
			const tuple &ax = t1[i];
			double a = ax.col1;
			double sum = 0;

			// Order t2, t3 or t3, t2 does not matter much according
			// to the benchmarks.
			for (const tuple &by : t2) {
				double b = by.col1;
				double c = t3.front().col1;
				// Bail early. From now on 'b' only grow and 'c'
				// here is minimal.
				if (double_ge(a, b + c))
					break;
				double xy = ax.col2 * by.col2;

				for (const tuple &cz : t3) {
					c = cz.col1;
					// Bail early. 'b' is fixated and 'c'
					// will only grow.
					if (double_ge(a, b + c))
						break;
					sum += xy * cz.col2;
				}
			}
			out[i] = {
				.row_id = ax.row_id,
				.col1 = ax.col1,
				.col2 = sum,
			};
		}
	}

	// Worker body end. ////////////////////

		});
	}
	for (int i = 0; i < worker_count; ++i)
		workers[i].join();
}

static void
cubic_execute(table &out, table &t1, table &t2, table &t3)
{
	if (t1.empty() || limit == 0)
		return;

	// Assume t1, t2, and t3 have T1, T2, T3 items. The limit is LIM.
	// Assume LIM < T1. Otherwise just replace it with T1 everywhere. For
	// complexity formulas.

	// Step 0: prepare data. ///////////////////////////////////////////////

	// Average complexity is T1*log(T1) + T2*log(T2) + T3*log(T3).
	// Worst case (if all is already sorted): T1^2 + T2^2 + T3^2 (due to
	// using 'quick sort', if STL still does that).

	// t1 is sorted for fast GROUP BY 'a'.
	table_sort_col1(t1);
	// t2 and t3 are sorted to enable 'early quit' optimization during SUM
	// calculation. See the implementation above.
	table_rsort_col1(t2);
	table_rsort_col1(t3);
	// Sorting is not parallel because it is almost nothing compared to the
	// SUM JOIN stage. According to benches.
	//
	// At the same time, having these sorted allows 'early quit'
	// optimization during the JOIN - when for some 'a' values no need to
	// scan the entire t2xt3 set. According to benchmarks, on random data
	// with T1 >> T2 and T3 it helps quite a lot. Despite it makes the
	// worst theoretical complexity potentially worse.

	// Step 1: prepare result. /////////////////////////////////////////////

	// Complexity: LIM.

	// Find result size.
	int t1_size = t1.size();
	int t1_last = t1_size - 1;
	int out_size = 1;
	for (int i = 0, next_i = 1; i < t1_last; i = next_i++) {
		out_size += double_neq(t1[i].col1, t1[next_i].col1);
		if (out_size == limit)
			break;
	}

	// Step 2: perform sum calculation for each t1 row. ////////////////////

	// Worst case: T1*T2*T3.
	// Average can't tell. The data is random. The internal loop by t2xt3
	// not always rolls all the way.

	// Could return the result right into t1, update it in place. But I
	// don't want to change the tables. That wouldn't be fair.
	table t_a_xyz;
	cubic_execute_sum(t_a_xyz, t1, t2, t3);

	// Step 3: group t1 rows. //////////////////////////////////////////////

	// Complexity: T1*log(LIM).

	// ORDER BY ... LIMIT is executed by walking the result set and putting
	// all into a heap of the LIMIT size. This way in the end there will be
	// exactly LIMIT target tuples. No need to sort all of them.
	//
	// Heap is minimal. But the only important part is 'pop' - it will
	// remove the smallest items. Hence, paradoxical, in the end it will
	// contain max items. However sorted ascending.
	heap_ge_col2 heap;
	heap.reserve(out_size + 1);

	// Aggregate equal 'a' values by merging them sums. Equal sums are
	// ordered STABLE, according to 'a' values appearance. For that need to
	// use the minimal row ID for each unique 'a' value.
	double sum = 0;
	int row_id = INT_MAX;
	for (int i = 0, next_i = 1; i < t1_size; i = next_i++) {
		const tuple &a_xyz = t_a_xyz[i];
		sum += a_xyz.col2;
		if (a_xyz.row_id < row_id)
			row_id = a_xyz.row_id;
		if (i < t1_last && double_eq(a_xyz.col1, t_a_xyz[next_i].col1))
			continue;

		heap.push({
			.row_id = row_id,
			.col1 = a_xyz.col1,
			.col2 = sum,
		});
		if (heap.size() > limit)
			heap.pop();
		sum = 0;
		row_id = INT_MAX;
	}

	// Step 4: dump result. ////////////////////////////////////////////////

	// Complexity: LIM.

	// Fill the result table in reversed order. Because the heap is minimal
	// while the result set should by descending.
	out.resize(out_size);
	for (int i = out_size - 1; i >= 0; --i) {
		const tuple &tr = heap.top();
		out[i] = {
			.row_id = i,
			.col1 = tr.col1,
			.col2 = tr.col2,
		};
		heap.pop();
	}

	// #### Total complexity average.
	//
	// T1*log(T1) + T2*log(T2) + T3*log(T3) + LIM + T1*T2*T3 + T1*log(LIM)
	//
	// If the tables are big enough, then it turns into T1*T2*T3.
	// In all formulas imagine they are in O(...). I didn't want to write it
	// everywhere.
	//
	// If the sorting in the beginning is removed, the complexity becomes
	// literally T1*T2*T3, not depending on these Tn*log(Tn). But makes
	// execution time actually worse on random unsorted data.
	//
	// Space complexity is T1*2 + T2 + T3 + LIM.
	//
	// #### Analysis, thinking of better complexity.
	//
	// Achieving something faster than T1*T2*T3 is possible. See the
	// quadratic solution below. But this cubic solution is optimal in terms
	// of memory usage. Can be reduced even further if, for example, store
	// t_a_xyz right in place of t1.
	//
	// If there would be many duplicates, it could make sense to 'compact'
	// t1 table - store number of duplicate rows for each original row.
	// The same could be true to t2 and t3 - compact them right in-place.
	// But there isn't any info about duplicates, so it makes little sense
	// in common case IMO.
	//
	// There is an alternative how to drop T1 sorting in the beginning and
	// do that in the end. For that would to sort t_a_xyz into a tree, where
	// equal 'a' are merged by SUM. But that does not look much better.
	// Sorting still would be T1*log(T1) average + tree rebalancing + it
	// will need to allocate each node individually. Looks worse than now.
}

//////////////////////////////////////////////////
// Quadratic solution. But takes a lot of space.
//

// Join t1 and t2 into a new table with rows:
//
//   [t1.col1 + t2.col1, t1.col2 * t2.col2]
//
static void
quadratic_merge(table &out, const table &tab1, const table &tab2)
{
	// The processing is done in parallel using steps over the external
	// loop. Hence the longer the external loop, the better threads will
	// split it between each other.
	// Otherwise, for example, could join tables of sizes 1kk and 1. If
	// would put '1' into the external loop, there wouldn't be any
	// parallelization.
	//
	// It does not give a big win on small data, but on > tens of thousands
	// of rows the difference is very clearly visible.
	//
	// Is not a game changer, like it is in the cubic solution.
	//
	const table *tab_small;
	const table *tab_big;
	int tab1_size = tab1.size();
	int tab2_size = tab2.size();
	if (tab1_size <= tab2_size) {
		tab_small = &tab1;
		tab_big = &tab2;
	} else {
		tab_small = &tab2;
		tab_big = &tab1;
	}
	out.resize(tab1_size * tab2_size);

	std::vector<std::thread> workers;
	workers.resize(worker_count);

	// Only need to sync the workers what ranges they take and where store
	// the results.
	//
	// The ranges are spread dynamically using an atomic counter. One by
	// one, first came - first served. That allows the threads evenly share
	// the load with the 'step_size' precision.
	//
	std::atomic<int> pos(0);
	for (int i = 0; i < worker_count; ++i) {
		workers[i] = std::thread([&]() {

	// Worker body start. ////////////////////

	int i1;
	int size = tab_big->size();
	while ((i1 = pos.fetch_add(step_size)) < size) {
		int i2 = std::min(size, i1 + step_size);
		int out_i = i1 * tab_small->size();
		for (int i = i1; i < i2; ++i) {
			const tuple &ti = tab_big->at(i);
			for (const tuple &tj : *tab_small) {
				out[out_i] = {
					.row_id = out_i,
					.col1 = ti.col1 + tj.col1,
					.col2 = ti.col2 * tj.col2,
				};
				out_i++;
			}
		}
	};

	// Worker body end. ////////////////////

		});
	}
	for (int i = 0; i < worker_count; ++i)
		workers[i].join();
}

// Sort the table by the first column. Then accumulate values of the second
// column in reversed order. For instance, the table:
//
//   [v5, v6]
//   [v1, v2]
//   [v3, v4]
//
// Turns into
//
//   [v1, v2 + v4 + v6]
//   [v3, v4 + v6]
//   [v5, v6]
//
static void
quadratic_reverse_accum_col2(table &tab)
{
	std::sort(tab.begin(), tab.end(), tuple_lt_col1);

	double sum = 0;
	int size = tab.size();
	for (int i = size - 1; i >= 0; --i) {
		tuple &t = tab[i];
		sum += t.col2;
		t.col2 = sum;
	}
}

static void
quadratic_execute(table &out, table &t1, table &t2, table &t3)
{
	if (t1.empty() || limit == 0)
		return;

	// Assume t1, t2, and t3 have T1, T2, T3 items. The limit is LIM.
	// Assume LIM < T1. Otherwise just replace it with T1 everywhere. For
	// complexity formulas.

	// Step 0: prepare data. ///////////////////////////////////////////////

	// Average complexity is T1*log(T1) + T2*T3*log(T2*T3).

	// t1 is sorted for fast GROUP BY 'a'.
	table_sort_col1(t1);

	// Build an accumulated join of t2 and t3 of form:
	//
	//   row_i = [t2.col1 + t3.col1, sum(t2.col2 * t3.col2)_i]
	//
	table t23acc;
	quadratic_merge(t23acc, t2, t3);
	quadratic_reverse_accum_col2(t23acc);

	// Step 1: prepare result. /////////////////////////////////////////////

	// Complexity: LIM.

	// Find result size.
	int t1_size = t1.size();
	int t1_last = t1_size - 1;
	int out_size = 1;
	for (int i = 0, next_i = 1; i < t1_last; i = next_i++) {
		out_size += double_neq(t1[i].col1, t1[next_i].col1);
		if (out_size == limit)
			break;
	}

	// Step 2: group t1 rows. //////////////////////////////////////////////

	// Complexity: T1*(log(LIM) + log(T2*T3)).

	// ORDER BY ... LIMIT is executed by walking the result set and putting
	// all into a heap of the LIMIT size. This way in the end there will be
	// exactly LIMIT target tuples. No need to sort all of them.
	//
	// Heap is minimal. But the only important part is 'pop' - it will
	// remove the smallest items. Hence, paradoxical, in the end it will
	// contain max items. However sorted ascending.
	heap_ge_col2 heap;
	heap.reserve(out_size + 1);

	// Aggregate equal 'a' values by merging them sums. Equal sums are
	// ordered STABLE, according to 'a' values appearance. For that need to
	// use the minimal row ID for each unique 'a' value.
	double sum = 0;
	int row_id = INT_MAX;
	for (int i = 0, next_i = 1; i < t1_size; i = next_i++) {
		const tuple &ax = t1[i];
		// Find the closest accumulated sum of 'y * z' so that
		// a < b + c. Then can multiply x on that sum and it would be
		// the same as SUM(x * y * z) on all of the matching rows.
		// But might be not found if a >= b + c everywhere.
		const tuple *bc_sum_yz = table_upper_bound_lt_col1(t23acc, ax);
		if (bc_sum_yz != nullptr)
			sum += ax.col2 * bc_sum_yz->col2;

		// Aggregation of equal 'a's.
		if (ax.row_id < row_id)
			row_id = ax.row_id;
		if (i < t1_last && double_eq(ax.col1, t1[next_i].col1))
			continue;

		heap.push({
			.row_id = row_id,
			.col1 = ax.col1,
			.col2 = sum,
		});
		if (heap.size() > limit)
			heap.pop();
		sum = 0;
		row_id = INT_MAX;
	}

	// Step 3: dump result. ////////////////////////////////////////////////

	// Complexity: LIM.

	// Fill the result table in reversed order. Because the heap is minimal
	// while the result set should by descending.
	out.resize(out_size);
	for (int i = out_size - 1; i >= 0; --i) {
		const tuple &tr = heap.top();
		out[i] = {
			.row_id = i,
			.col1 = tr.col1,
			.col2 = tr.col2,
		};
		heap.pop();
	}

	// #### Total complexity average.
	//
	// T1*log(T1) + T2*T3*log(T2*T3) + T1*(log(LIM) + log(T2*T3))
	//
	// Use log(a)+log(b) = log(ab) formula and get this:
	//
	// T2*T3*log(T2*T3) + T1*log(T1*T2*T3*LIM)
	//
	// It is not exactly quadratic, but still -1 order compared to the
	// cubic solution.
	//
	// Space complexity is T1 + T2 + T3 + T2*T3 + LIM.
	//
	// #### Analysis, thinking of better complexity.
	//
	// This solution needs a temporary storage to store t2*t3 accumulated
	// join. This works fine as long as it fits into the memory, but could
	// degrade drastically if would need to use disk as a buffer.
	//
	// The current solution uses std::sort, which is probably quick sort
	// and would give quadratic sorting complexity on already sorted data.
	// It could be possible to sacrifice a bit more space and use heap-sort
	// to make it more 'stable'. However can't tell whether it will be
	// really faster without trying.
	//
	// Another option would be to save sorted tuples into std::map which is
	// an RB-tree instead of sorting them as arrays. But it wasn't done
	// because in the map can't reserve space. It seems it would allocate a
	// new memory block for each inserted tuple, which gives T1 + T2*T3
	// new/delete calls to fill these maps and then destroy them. Huge
	// memory fragmentation.
}

//////////////////////////////////////////////////
// Real DBs.
//
// The same query can be checked in real databases. For instance, EXPLAIN
// command in sqlite shows the same complexity. They merge t2xt3 into a
// temporary table, then walk t1 and check it for each row in t2xt3. It gives
// T1*T2*T3. Although they also do some sorting to be able to return rows one by
// one. So it is actually worse.
//
// In MySQL they seem to do this: sort t1, then for each t1 row they walk t2 and
// t3. That gives T1*log(T1) + T1*T2*T3, closer to the cubic solution.
//
// In PostgreSQL I couldn't get this query compile at all.

using execute_f = void(*)(table &out, table &t1, table &t2, table &t3);

static void
execute_and_write(execute_f exec, const char *file_res, const char *file1,
	const char *file2, const char *file3)
{
	table t1, t2, t3, res;
	table_read(t1, file1);
	table_read(t2, file2);
	table_read(t3, file3);
	exec(res, t1, t2, t3);
	table_write(res, file_res);
}

static void
test(execute_f exec, int id)
{
	std::cout << "Test " << id << ' ';
	const int path_size = 128;
	char dir[path_size];
	char t1_path[path_size];
	char t2_path[path_size];
	char t3_path[path_size];
	char res_path[path_size];
	const char *real_path = "real";
	snprintf(dir, path_size, "./tests/%d", id);
	snprintf(t1_path, path_size, "%s/t1", dir);
	snprintf(t2_path, path_size, "%s/t2", dir);
	snprintf(t3_path, path_size, "%s/t3", dir);
	snprintf(res_path, path_size, "%s/res", dir);

	execute_and_write(exec, real_path, t1_path, t2_path, t3_path);

	table exp, got;
	table_read(exp, res_path);
	table_read(got, real_path);

	if (exp.size() != got.size()) {
		std::cout << "size mismatch\n";
		goto crash;
	}
	for (int i = 0, size = exp.size(); i < size; ++i) {
		const tuple &l = exp[i];
		const tuple &r = got[i];
		if (l.row_id != r.row_id || l.col1 != r.col1 ||
		l.col2 != r.col2) {
			std::cout << "row " << i + 1 << " mismatch\n";
			goto crash;
		}
	}
	std::cout << "passed\n";
	return;

crash:
	std::cout << "failed\n";
	std::cout << "expected:\n";
	table_print(exp);
	std::cout << "real:\n";
	table_print(got);
	abort();
}

static void
bench(execute_f exec)
{
	const char *res_file = "real";
	struct timespec ts1, ts2;
	clock_gettime(CLOCK_MONOTONIC, &ts1);

	execute_and_write(exec, res_file, "t1", "t2", "t3");

	clock_gettime(CLOCK_MONOTONIC, &ts2);

	uint64_t usec1 = ts1.tv_sec * 1000000 + ts1.tv_nsec / 1000;
	uint64_t usec2 = ts2.tv_sec * 1000000 + ts2.tv_nsec / 1000;
	std::cout << "bench took " << timespec_diff_us(ts1, ts2) << " us\n";

	remove(res_file);
}

static void
run_suite(const char *name, execute_f exec)
{
	std::cout << "Suite " << name << '\n';
	for (int i = 1; i <= 5; ++i)
		test(exec, i);
	bench(exec);
}

int
main()
{
	run_suite("quadratic", quadratic_execute);
	run_suite("cubic", cubic_execute);
	return 0;
}
