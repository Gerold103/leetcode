# Coulomb solver

The program takes a system of particles with their coordinates + charges, and calculates Coulomb forces affecting each of the particles. It also can optionally calculate particle potentials and energy of the whole system.

For the usage options try `-h`:
```
$ ./solver_exe -h
    -i - Input file name. Default is 'input.dat'.
    -o - Output file name. Default is 'output.dat'.

    -t - Thread count. Threads try to fairly distribute the calculations for
        different particles. Default is the logical CPU core count.

    -s - Show calculation progress in percents. Helpful to see that the program
        didn't hang.

    -p - Calculate potential for each particle.
    -e - Calculate system energy.
    -h - Show this message.
```

## Build

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j
# Run tests
./test/test_exe
```

## Running
```
./solver_exe -p -e -s
```
This command reads input from `input.dat`, writes result to `output.dat`. It will calculate particle potentials (`-p`) and write them as 4th column in the result file. It will also calculate system energy (`-e`). While the calculation is going, you will see progress printed in percents (`-s`).

Output example:
```
Parameters:
	Thread count: 12
	With potentials
	With system energy
Read the file "input.dat"
Reading the file took 0.0915186 seconds
Got 114537 particles
Start computation
1%
2%
3%
4%
...
96%
97%
98%
99%
The computation took 14.5219 seconds
System energy: 9.97386e+11
```
