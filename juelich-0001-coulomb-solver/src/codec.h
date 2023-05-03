#pragma once

struct particle_system;

// Load particle system from a text file. The file can have comment-lines
// starting from '#' symbol. The data-lines should have 4 columns with numbers:
// particle charge, x, y, z coordinates.
bool
particle_system_parse_dat_file(
	particle_system& out,
	const char *file);

// Dump system result properties to a text file. Each line has 3 columns + 1
// optional. Force vector (x, y, z parts) and particle potential.
bool
particle_system_dump_result_dat_file(
	const particle_system& sys,
	unsigned flags,
	const char *file);
