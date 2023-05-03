#include "unit.h"

#include "codec.h"
#include "coulomb.h"
#include "utils.h"

#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

static void
dump_to_file(
	const char *file,
	const char *data);

static void
parse_result_dat_file(
	particle_system& out,
	const char *file);

static void
check_solution(
	const particle_system& got,
	const particle_system& expected,
	unsigned flags);

static void
solve_and_check(
	const char *system,
	const char *result,
	double energy,
	const solution_params& params);

////////////////////////////////////////////////////////////////////////////////

static void
test_double_is_eq()
{
	unit_test_start();

	unit_check(double_is_eq(1, 1), "1 vs 1");
	unit_check(!double_is_eq(1.5, 1), "1.5 vs 1");
	unit_check(double_is_eq(0.0001, 0.0001), "0.0001 vs 0.0001");
	unit_check(!double_is_eq(0.0001, 0.0002), "0.0001 vs 0.0002");

	unit_test_finish();
}

static void
test_string_to_num_uint()
{
	unit_test_start();

	char buf[128];
	sprintf(buf, "%llu", (long long)UINT_MAX + 1);
	unsigned num;
	unit_check(!string_to_num(buf, num), "too big");
	unit_check(!string_to_num("-123", num), "negative");
	unit_check(!string_to_num("", num), "empty");

	num = 123;
	unit_check(string_to_num("0", num) && num == 0, "0");
	unit_check(string_to_num("123", num) && num == 123, "123");
	unit_check(string_to_num(" \t 34 \n ", num) && num == 34, "34 with spaces");
	unit_check(!string_to_num(" 34 ab ", num), "34 with garbage after space");
	unit_check(!string_to_num(" 34ab ", num), "34 + garbage");
	unit_check(!string_to_num(" ab34 ", num), "garbage + 34");

	sprintf(buf, "%u", UINT_MAX);
	unit_check(string_to_num(buf, num) && num == UINT_MAX, "max");

	unit_test_finish();
}

static void
test_string_read_num_double()
{
	unit_test_start();

	double num;
	const char *str;
	unit_check(!string_read_num((str = ""), num), "empty");
	unit_check(!string_read_num((str = "abc"), num), "garbage");

	num = 123;
	unit_check(string_read_num((str = "0"), num) &&
		double_is_eq(num, 0), "0");
	unit_check(string_read_num((str = "123.5"), num) &&
		double_is_eq(num, 123.5), "123.5");

	unit_check(string_read_num((str = " \t 34.7 \n "), num) &&
		double_is_eq(num, 34.7), "34.7 with spaces");
	unit_check(strcmp(str, " \n ") == 0, "str is moved");

	unit_check(string_read_num((str = " 56.3 ab "), num) &&
		double_is_eq(num, 56.3), "56.3 with garbage after space");
	unit_check(strcmp(str, " ab ") == 0, "str is moved");

	unit_check(!string_read_num((str = " 34ab "), num),
		"34 + garbage");
	unit_check(!string_read_num((str = " ab34 "), num),
		"garbage + 34");

	unit_test_finish();
}

////////////////////////////////////////////////////////////////////////////////

static void
test_2_particles()
{
	unit_test_start();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	solve_and_check(
R"--(
52	11	38	11
57	16	81	94)--",

R"--(
-0.01806629 -0.155370097 -0.29990042 0.608903471
0.01806629 0.155370097 0.29990042 0.555490886)--",
	31.662980497,
	params);

	unit_test_finish();
}

static void
test_3_particles()
{
	unit_test_start();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	solve_and_check(
R"--(
59	59	19	48
48	59	21	60
71	39	66	57)--",

R"--(
0.600498513 -4.556968343 -19.14500536 5.314508745
0.567608278 1.868678212 18.959922271 6.288893615
-1.168106791 2.688290131 0.185083089 2.11049264)--",
	382.633943458,
	params);

	unit_test_finish();
}

static void
test_4_particles()
{
	unit_test_start();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	solve_and_check(
R"--(
24	48	39	93
84	88	12	42
58	59	56	3
12	89	32	7)--",

R"--(
-0.266008242 0.129416486 0.485016389 1.950686768
0.719010228 -1.225504686 0.915387638 1.524101661
-0.84422745 1.08077166 -0.882220618 1.85211778
0.391225464 0.01531654 -0.518183409 3.835935472)--",
	164.14753943,
	params);

	unit_test_finish();
}

static void
test_5_particles()
{
	unit_test_start();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	solve_and_check(
R"--(
24	48	39	93
84	88	12	42
58	59	56	3
12	89	32	7)--",

R"--(
-0.2660082423 0.1294164861 0.4850163889 1.9506867676
0.7190102281 -1.2255046863 0.9153876377 1.5241016610
-0.8442274499 1.0807716604 -0.8822206177 1.8521177798
0.3912254641 0.0153165398 -0.5181834088 3.8359354718)--",
	164.14753941689568,
	params);

	unit_test_finish();
}

static void
test_25_particles()
{
	unit_test_start();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	solve_and_check(
R"--(
14	51	69	16
12	82	65	1
35	76	62	49
48	85	62	42
100	14	78	92
96	30	98	47
87	31	29	47
8	8	34	49
3	96	5	70
65	97	75	97
71	67	46	99
69	49	21	16
79	40	67	33
11	58	75	60
4	47	99	26
76	41	94	90
42	32	4	30
65	6	92	31
18	21	60	77
65	70	24	62
20	15	76	72
96	31	75	10
83	12	3	93
1	82	65	29
62	72	16	75)--",

R"--(
4.2753709492 0.1112752475 -3.6242132097 24.7435242603
1.5364502079 0.2585819575 -1.8388616073 16.4140747365
-3.4897908969 1.4291423905 6.6546622712 24.0936942894
18.7826953176 1.6743023528 -10.1119419566 20.9867021552
-15.4605889332 3.5117836897 18.5005391192 17.9709524160
0.1875556640 20.4328957727 1.7135113004 19.8616246031
-8.0719880872 -8.1058396596 1.1314723886 20.7569702260
-2.3409819623 -0.4411354111 -0.0250398400 21.6369947490
0.6279668658 -0.4705798122 0.0506549886 16.8588819775
8.7087515555 4.0023428552 5.2981929400 14.7755549254
3.6368062170 -1.5805245468 11.6781343591 18.0213933778
4.4253796455 -7.0655269758 -12.8160184943 18.1325956183
2.7082395261 -2.6262969755 0.5818184458 23.6615437853
1.1197706445 1.1093956181 0.8596734600 24.6525671329
0.6458460626 0.9866312365 -0.6117728888 22.0610497054
6.0072502949 12.1246404056 9.8311795992 18.6044139241
-3.5807767262 -10.2111388334 -3.2824663358 18.4804919543
-15.5525984636 6.8100272890 -6.0499850980 18.8195932205
-1.0719355444 -3.2120093812 0.6878435134 24.1540794272
5.4086386994 0.0957534201 -11.9442365185 21.7827618572
-3.3282656820 0.5425023954 -2.7234569068 24.7053273267
-7.0653690736 5.7911778856 -26.7595592298 19.1025919647
-6.0817526275 -8.2933569123 6.4793972117 13.7604104897
0.1479512705 0.0998043968 -0.3771436113 22.7439846880
7.8253750765 -16.9738484052 16.6976160995 20.2342243102)--",
	11929.25045943592,
	params);

	unit_test_finish();
}

static void
test_stress(
	const solution_params& params)
{
	unit_test_start();

	unit_msg("Using %u threads", params.thread_count);
	if ((params.flags & COULOMB_NEED_POTENTIALS) != 0)
		unit_msg("With potentials");
	if ((params.flags & COULOMB_NEED_SYSTEM_ENERGY) != 0)
		unit_msg("With system energy");

	solve_and_check(
#include "testStress300System.dat"
,
#include "testStress300Result.dat"
,
	17444519507.812099,
	params);

	unit_test_finish();
}

int
main()
{
	unit_test_start();
	//
	// Common tests.
	//
	test_double_is_eq();
	test_string_to_num_uint();
	test_string_read_num_double();
	//
	// Coulomb tests.
	//
	test_2_particles();
	test_3_particles();
	test_4_particles();
	test_5_particles();
	test_25_particles();

	solution_params params;
	params.thread_count = 1;
	params.flags = COULOMB_NEED_POTENTIALS | COULOMB_NEED_SYSTEM_ENERGY;
	test_stress(params);

	params.thread_count = 5;
	test_stress(params);

	params.flags = 0;
	test_stress(params);

	params.flags = COULOMB_NEED_POTENTIALS;
	test_stress(params);

	params.flags = COULOMB_NEED_SYSTEM_ENERGY;
	test_stress(params);

	unit_test_finish();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

static void
dump_to_file(
	const char *file,
	const char *data)
{
	int fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		std::cout << "Couldn't open test file \"" << file
			<< "\" for writing - " << strerror(errno) << "\n";
		exit(-1);
	}
	ssize_t len = strlen(data);
	if (write(fd, data, len) != len) {
		std::cout << "Couldn't write test data\n";
		exit(-1);
	}
	close(fd);
}

static void
parse_result_dat_file(
	particle_system& out,
	const char *file)
{
	char *line = nullptr;
	size_t lineSize = 0;
	ssize_t rc;
	unsigned lineIdx = 1;
	size_t lineCount = 0;
	std::vector<particle>& particles = out.particles;
	FILE* fd = fopen(file, "r");
	unit_assert(fd != nullptr);
	while ((rc = getline(&line, &lineSize, fd)) > 0) {
		++lineCount;
		unit_assert(lineCount <= COULOMB_MAX_PARTICLE_COUNT);
	}
	rewind(fd);
	particles.reserve(lineCount);
	for (; (rc = getline(&line, &lineSize, fd)) > 0; ++lineIdx) {
		const char *pos = line;
		while (isspace(*pos))
			++pos;
		if (*pos == 0)
			continue;
		//
		// Force components.
		//
		size_t idx = particles.size();
		particles.resize(idx + 1);
		particle& p = particles[idx];
		unit_assert(string_read_num(pos, p.force_x));
		unit_assert(string_read_num(pos, p.force_y));
		unit_assert(string_read_num(pos, p.force_z));
		while (isspace(*pos))
			++pos;
		if (*pos == 0)
			continue;
		//
		// Optional potential.
		//
		unit_assert(string_read_num(pos, p.potential));
		while (isspace(*pos))
			++pos;
		// End of line.
		unit_assert(*pos == 0);
	}
	unit_assert(feof(fd));
	free(line);
	fclose(fd);
}

static void
check_solution(
	const particle_system& got,
	const particle_system& expected,
	unsigned flags)
{
	bool need_energy = (flags & COULOMB_NEED_SYSTEM_ENERGY) != 0;
	bool need_potentials = (flags & COULOMB_NEED_POTENTIALS) != 0;
	unit_assert(!need_energy || double_is_eq(got.energy, expected.energy));

	size_t gotCount = got.particles.size();
	size_t expCount = expected.particles.size();
	unit_assert(gotCount == expCount);

	for (size_t i = 0; i < gotCount; ++i) {
		const particle& gotP = got.particles[i];
		const particle& expP = expected.particles[i];
		unit_assert(
			double_is_eq(gotP.force_x, expP.force_x) &&
			double_is_eq(gotP.force_y, expP.force_y) &&
			double_is_eq(gotP.force_z, expP.force_z));
		unit_assert(!need_potentials ||
			double_is_eq(gotP.potential, expP.potential));
	}
}

static void
solve_and_check(
	const char *system,
	const char *result,
	double energy,
	const solution_params& params)
{
	const char *in_file = "system.dat";
	dump_to_file(in_file, system);

	unit_msg("Read in-file");
	particle_system sys;
	unit_assert(particle_system_parse_dat_file(sys, in_file));
	sys.solve(params);

	unit_msg("Dump result");
	const char *out_file = "got.dat";
	unit_assert(particle_system_dump_result_dat_file(
		sys, params.flags, out_file));

	unit_msg("Read real result");
	particle_system sys_from_file;
	sys_from_file.energy = energy;
	parse_result_dat_file(sys_from_file, out_file);

	unit_msg("Read expected result");
	const char *exp_file = "expected.dat";
	dump_to_file(exp_file, result);
	particle_system sys_expected;
	sys_expected.energy = energy;
	parse_result_dat_file(sys_expected, exp_file);

	unit_msg("Check the file dump");
	check_solution(sys_from_file, sys, params.flags);

	unit_msg("Check the solution correctness");
	check_solution(sys, sys_expected, params.flags);
}
