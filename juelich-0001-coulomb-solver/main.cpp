#include <cassert>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "codec.h"
#include "coulomb.h"
#include "utils.h"

int
main(
	int argc,
	char **argv)
{
	solution_params params;
	params.flags = 0;
	params.thread_count = std::thread::hardware_concurrency();
	const char *in_file = "input.dat";
	const char *out_file = "output.dat";

	const char *opts = "i:o:t:speh";
	int c;
	while ((c = getopt(argc, argv, opts)) != -1) {
		switch(c) {
		case 'i':
			in_file = optarg;
			break;
		case 'o':
			out_file = optarg;
			break;
		case 't':
			if (!string_to_num(optarg, params.thread_count)) {
				std::cout << "Invalid thread count number\n";
				return -1;
			}
			if (params.thread_count > COULOMB_MAX_THREAD_COUNT || params.thread_count == 0) {
				std::cout << "Thread count is outside of the range [1, " << COULOMB_MAX_THREAD_COUNT << "]\n";
				return -1;
			}
			break;
		case 's':
			params.flags |= COULOMB_NEED_PROGRESS;
			break;
		case 'p':
			params.flags |= COULOMB_NEED_POTENTIALS;
			break;
		case 'e':
			params.flags |= COULOMB_NEED_SYSTEM_ENERGY;
			break;
		case 'h':
			std::cout <<
R"--(## Coulomb solver ##

The program takes a system of particles with their coordinates + charges, and
calculates Coulomb forces affecting each of the particles. It also can
optionally calculate particle potentials and energy of the whole system.

    -i - Input file name. Default is 'input.dat'.
    -o - Output file name. Default is 'output.dat'.

    -t - Thread count. Threads try to fairly distribute the calculations for
        different particles. Default is the logical CPU core count.

    -s - Show calculation progress in percents. Helpful to see that the program
        didn't hang.

    -p - Calculate potential for each particle.
    -e - Calculate system energy.
    -h - Show this message.)--" << "\n";
			return 0;
		default:
			std::cout << "Invalid parameters\n";
			return -1;
		}
	}
	std::cout << "Parameters:\n";
	std::cout << "\tThread count: " << params.thread_count << "\n";
	if ((params.flags & COULOMB_NEED_POTENTIALS) != 0)
		std::cout << "\tWith potentials\n";
	if ((params.flags & COULOMB_NEED_SYSTEM_ENERGY) != 0)
		std::cout << "\tWith system energy\n";


	std::cout << "Read the file \"" << in_file << "\"\n";
	particle_system sys;
	int64_t ts_nano1 = time_monotonic_nanosec();
	if (!particle_system_parse_dat_file(sys, in_file)) {
		std::cout << "Couldn't read the file\n";
		return -1;
	}
	uint64_t ts_nano2 = time_monotonic_nanosec();
	double sec = (ts_nano2 - ts_nano1) / 1000000000.0;
	std::cout << "Reading the file took " << sec << " seconds\n";
	std::cout << "Got " << sys.particles.size() << " particles\n";

	std::cout << "Start computation\n";
	ts_nano1 = time_monotonic_nanosec();
	sys.solve(params);
	ts_nano2 = time_monotonic_nanosec();
	sec = (ts_nano2 - ts_nano1) / 1000000000.0;
	std::cout << "The computation took " << sec << " seconds\n";

	if ((params.flags & COULOMB_NEED_SYSTEM_ENERGY))
		std::cout << "System energy: " << sys.energy << "\n";
	if (!particle_system_dump_result_dat_file(sys, params.flags, out_file)) {
		std::cout << "Couldn't write the file\n";
		return -1;
	}
	return 0;
}
