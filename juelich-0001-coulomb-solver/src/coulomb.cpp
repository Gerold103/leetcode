#include "coulomb.h"

#include "utils.h"

#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

particle::particle()
{
	memset(this, 0, sizeof(*this));
}

particle_system::particle_system()
	: energy(0)
{
}

struct solution_worker_ctx {
	solution_worker_ctx(
		particle_system& sys_);

	std::vector<particle>& particles;

	// False-sharing protection.
	uint8_t padding[COULOMB_CACHE_LINE_SIZE];

	std::atomic<unsigned> index;
};

// Calculate solution for one given particle against all the other particles.
template<bool need_potential>
static void
solve_one_particle(
	std::vector<particle>& particles,
	unsigned p_idx)
{
	particle& p_i = particles[p_idx];
	unsigned count = particles.size();
	double force_x = 0;
	double force_y = 0;
	double force_z = 0;
	double potential = 0;
	for (unsigned j = 0; j < count; ++j) {
		if (j == p_idx)
			continue;
		const particle& p_j = particles[j];
		double x_dist = p_i.x - p_j.x;
		double y_dist = p_i.y - p_j.y;
		double z_dist = p_i.z - p_j.z;
		double dist_pow2 = x_dist * x_dist + y_dist * y_dist + z_dist * z_dist;
		double dist = sqrt(dist_pow2);
		double dist_pow3 = dist_pow2 * dist;
		// It is tempting to save and reuse the `charge / dist_pow3` value, but
		// worsens the precision too much.
		force_x += p_j.charge * x_dist / dist_pow3;
		force_y += p_j.charge * y_dist / dist_pow3;
		force_z += p_j.charge * z_dist / dist_pow3;
		if constexpr (need_potential)
			potential += p_j.charge / dist;
	}
	force_x *= p_i.charge;
	force_y *= p_i.charge;
	force_z *= p_i.charge;
	// Note that the solution is written not into the particle object right
	// away. It is saved into local variables on the stack which are then
	// flushed into the particle object below.
	// The reason is false-sharing protection. If all the threads would
	// constantly modify the particle objects to update their members, the
	// threads would too often spoil the cache lines for each other. Ideally
	// each particle should be updated just once, with the final solution. Like
	// done here.
	p_i.force_x = force_x;
	p_i.force_y = force_y;
	p_i.force_z = force_z;
	if constexpr (need_potential)
		p_i.potential = potential;
}

template<bool need_potential, bool need_progress>
static void
solution_worker_f(
	solution_worker_ctx& ctx)
{
	std::vector<particle>& particles = ctx.particles;
	std::atomic<unsigned>& index = ctx.index;
	unsigned count = particles.size();
	while (true) {
		unsigned idx = index.fetch_add(1, std::memory_order_relaxed);
		if (idx >= count)
			break;
		if constexpr (need_progress) {
			unsigned prev_idx = idx > 0 ? idx - 1 : 0;
			unsigned prev_perc = prev_idx * 100 / count;
			unsigned cur_perc = idx * 100 / count;
			if (prev_perc != cur_perc)
				std::cout << cur_perc << "%\n";
		}
		solve_one_particle<need_potential>(particles, idx);
	}
}

void
particle_system::solve(
	const solution_params& params)
{
	void (*worker_f)(solution_worker_ctx&) = nullptr;
	bool need_progress = (params.flags & COULOMB_NEED_PROGRESS) != 0;
	// If need system energy, then also need potentials anyway. They are used in
	// the energy formula.
	bool need_potentials = (params.flags &
		(COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY)) != 0;

	if (need_potentials) {
		if (need_progress)
			worker_f = solution_worker_f<true, true>;
		else
			worker_f = solution_worker_f<true, false>;
	} else {
		if (need_progress)
			worker_f = solution_worker_f<false, true>;
		else
			worker_f = solution_worker_f<false, false>;
	}
	solution_worker_ctx ctx(*this);
	std::vector<std::thread*> threads;
	for (unsigned i = 0; i < params.thread_count; ++i)
		threads.push_back(new std::thread(worker_f, std::ref(ctx)));
	for (std::thread* t : threads) {
		t->join();
		delete t;
	}
	if ((params.flags & COULOMB_NEED_SYSTEM_ENERGY) != 0) {
		// Maybe on huge particle count it would make sense to let the worker
		// threads calculate parts of the sum, but for the trivial solver like
		// this one it wouldn't give any win.
		energy = 0;
		for (particle& p : particles)
			energy += p.charge * p.potential;
		energy /= 2;
	}
}

//////////////////////////////////////////////////////////////////

solution_worker_ctx::solution_worker_ctx(
	particle_system& sys_)
	: particles(sys_.particles)
	, index(0)
{
}
