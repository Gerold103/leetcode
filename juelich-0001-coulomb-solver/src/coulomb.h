#pragma once

#include <vector>

enum {
	// Calculate for each particle its potential. In addition to force.
	COULOMB_NEED_POTENTIALS = 1 << 0,
	// Calculate energy of the whole system.
	COULOMB_NEED_SYSTEM_ENERGY = 1 << 1,
	// Whether need to print calculation progress in percents.
	COULOMB_NEED_PROGRESS = 1 << 2,
};

struct solution_params {
	unsigned flags;
	unsigned thread_count;
};

struct particle {
	particle();

	// Input params.
	double charge;
	double x;
	double y;
	double z;

	// Solution.
	double force_x;
	double force_y;
	double force_z;
	double potential;
};

struct particle_system {
	particle_system();

	void
	solve(
		const solution_params& params);

	double energy;
	std::vector<particle> particles;
};
