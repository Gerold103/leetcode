#include <vector>
#include <map>
#include <iostream>

using IDList = std::vector<uint64_t>;

// User's definition of task. Does not contain any internals of the algorithm.
// Only definition.
struct TaskDef
{
	uint64_t id;
	IDList deps;
};

// Wrapper for user's task which has additional internal data.
struct Task
{
	Task()
		: m_def(nullptr)
		, m_ins(0)
		, m_next(nullptr)
		, m_prev(nullptr)
	{
	}

	void
	create(const TaskDef *def)
	{
		m_def = def;
		m_ins = def->deps.size();
	}

	const TaskDef *m_def;
	std::vector<Task *> m_outs;
	int m_ins;
	Task *m_next;
	Task *m_prev;
};

// Stack with ability to drop elements from the middle in constant time.
struct TaskStack
{
	TaskStack() : m_top(nullptr) { }

	void
	push(Task *task)
	{
		task->m_prev = nullptr;
		task->m_next = m_top;
		if (m_top != nullptr)
			m_top->m_prev = task;
		m_top = task;
	}

	void
	remove(Task *task)
	{
		Task *prev = task->m_prev;
		Task *next = task->m_next;
		if (prev != nullptr)
			prev->m_next = next;
		if (next != nullptr)
			next->m_prev = prev;
		if (task == m_top)
			m_top = next;
	}

	Task *
	pop()
	{
		Task *res = m_top;
		if (res == nullptr)
			return nullptr;
		m_top = res->m_next;
		res->m_next = nullptr;
		if (m_top != nullptr)
			m_top->m_prev = nullptr;
		return res;
	}

	Task *m_top;
};

static bool
execute(const TaskDef* defs, int def_count, IDList& out)
{
	int task_count = def_count;
	out.clear();
	out.reserve(task_count);
	Task *tasks = new Task[task_count];
	bool rc = true;

	// Need to be able to look up tasks by ID to build bi-directional
	// dependency links.
	std::map<uint64_t, Task *> map;

	// The key idea is that on each execution step can only execute a task
	// having no dependencies left. It could be represented as a graph with
	// dependencies as directed edges. Then would need to execute nodes
	// having no edges coming into them.
	// But also could represent it as an array of lists where a task belongs
	// to array[i] if its dependency count is 'i'. Then the complexity of
	// task update in such a 'graph' is constant.
	// For that the array items must be intrusive lists so as removal of a
	// task would be just a couple of link updates.
	std::vector<TaskStack> graph;
	graph.resize(task_count);

	// Populate the graph and create the task -> id mapping.
	for (int i = 0; i < task_count; ++i) {
		Task *t = &tasks[i];
		const TaskDef *d = &defs[i];
		t->create(d);
		graph[t->m_ins].push(t);
		map[d->id] = t;
	}
	// Having all the tasks in the map, now can find reversed dependencies.
	// So called 'fan-outs'.
	for (int i = 0; i < task_count; ++i) {
		Task *t = &tasks[i];
		for (uint64_t id : t->m_def->deps)
			map[id]->m_outs.push_back(t);
	}
	while (task_count > 0) {
		Task *t = graph[0].pop();
		// If have no tasks without deps, it means a cyclic dependency.
		if (t == nullptr) {
			rc = false;
			goto end;
		}
		out.push_back(t->m_def->id);
		// Decrease number of dependencies for all the tasks which had
		// this one as a dependency.
		for (Task *t : t->m_outs) {
			assert(t->m_ins > 0);
			graph[t->m_ins].remove(t);
			graph[--t->m_ins].push(t);
		}
		--task_count;
	}
end:
	delete[] tasks;
	return rc;
}

// assert(), but works in release build too.
static inline void
check(bool ok)
{
	if (!ok)
		abort();
}

int
main()
{
	IDList res;
	{
		TaskDef defs[1];
		defs[0].id = 1;
		// 1
		check(execute(defs, 1, res));
		check(res == IDList({1}));
	}
	{
		TaskDef defs[3];
		defs[0].id = 1;
		defs[1].id = 2;
		defs[1].deps = {1};
		defs[2].id = 3;
		defs[2].deps = {1, 2};
		// 1, 2, 3
		check(execute(defs, 3, res));
		check(res == IDList({1, 2, 3}));
	}
	{
		TaskDef defs[5];
		defs[0].id = 1;
		defs[0].deps = {2, 3, 4};
		defs[1].id = 2;
		defs[1].deps = {4};
		defs[2].id = 3;
		defs[2].deps = {2, 4};
		defs[3].id = 4;
		defs[3].deps = {5};
		defs[4].id = 5;
		// 5, 4, 2, 3, 1
		check(execute(defs, 5, res));
		check(res == IDList({5, 4, 2, 3, 1}));
	}
	{
		TaskDef defs[3];
		defs[0].id = 1;
		defs[0].deps = {2};
		defs[1].id = 2;
		defs[1].deps = {1};
		defs[2].id = 3;
		check(!execute(defs, 3, res));
	}
	return 0;
}
