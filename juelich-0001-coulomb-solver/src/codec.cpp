#include "codec.h"

#include "coulomb.h"
#include "utils.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>

bool
particle_system_parse_dat_file(
	particle_system& out,
	const char *file)
{
	char *line = nullptr;
	size_t line_size = 0;
	ssize_t rc;
	unsigned line_idx = 1;
	bool ok = true;
	size_t line_count = 0;
	std::vector<particle>& particles = out.particles;
	FILE* fd = fopen(file, "r");
	if (fd == nullptr) {
		std::cout << "Couldn't open file \"" << file
			<< "\" for reading - " << strerror(errno) << "\n";
		goto error;
	}
	while ((rc = getline(&line, &line_size, fd)) > 0) {
		const char *pos = line;
		while (isspace(*pos))
			++pos;
		// Skip comments and empty lines.
		if (*pos == '#' || *pos == 0)
			continue;
		++line_count;
		if (line_count > COULOMB_MAX_PARTICLE_COUNT) {
			std::cout << "Too many lines in the file (> "
				<< COULOMB_MAX_PARTICLE_COUNT << ")\n";
			goto error;
		}
	}
	rewind(fd);
	particles.reserve(line_count);
	for (; (rc = getline(&line, &line_size, fd)) > 0; ++line_idx) {
		const char *pos = line;
		while (isspace(*pos))
			++pos;
		// Skip comments and empty lines.
		if (*pos == '#' || *pos == 0)
			continue;

		size_t idx = particles.size();
		particles.resize(idx + 1);
		particle& p = particles[idx];
		ok = ok && string_read_num(pos, p.charge);
		ok = ok && string_read_num(pos, p.x);
		ok = ok && string_read_num(pos, p.y);
		ok = ok && string_read_num(pos, p.z);
		if (!ok) {
			std::cout << "Failed to parse line "
				<< line_idx << "\n";
			goto error;
		}
		while (isspace(*pos))
			++pos;
		if (*pos != 0) {
			std::cout << "Found trailing symbols at line "
				<< line_idx << "\n";
			goto error;
		}
	}
	if (!feof(fd)) {
		std::cout << "Failed to read the file on line "
			<< line_idx << "\n";
		goto error;
	}
finish:
	free(line);
	fclose(fd);
	return ok;
error:
	ok = false;
	goto finish;
}

bool
particle_system_dump_result_dat_file(
	const particle_system& sys,
	unsigned flags,
	const char *file)
{
	bool ok = true;
	bool need_potentials = (flags & COULOMB_NEED_POTENTIALS) != 0;
	FILE* fd = fopen(file, "w");
	if (fd == nullptr) {
		std::cout << "Couldn't open file \"" << file
			<< "\" for writing - " << strerror(errno) << "\n";
		goto error;
	}
	for (const particle& p : sys.particles) {
		if (need_potentials) {
			ok = fprintf(fd, "%.10lf %.10lf %.10lf %.10lf\n",
				p.force_x, p.force_y, p.force_z,
				p.potential) > 0;
		} else {
			ok = fprintf(fd, "%.10lf %.10lf %.10lf\n",
				p.force_x, p.force_y, p.force_z) > 0;
		}
		if (!ok) {
			std::cout << "Couldn't write a particle to the file\n";
			goto error;
		}
	}
finish:
	fclose(fd);
	return ok;
error:
	ok = false;
	goto finish;
}
