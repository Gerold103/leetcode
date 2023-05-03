# A Classical Coulomb Solver
In modern molecular dynamics (MD) simulations a fair amount of computation time is spent to repeatedly evaluate pairwise interaction between particles. In this exercise we want to implement a simple Coulomb solver in C++.

## Problem Description
MD simulations need to compute interactions of a set of $N$ particles. For long-range interactions, namely Coulomb forces and Coulomb potentials this is especially adverse, since the number of interactions is not limited to particles in the proximity. To obtain physically-consistent results in a simulation all pairwise interactions have to be taken into account. The aim of this exercise is to implement such a Coulomb solver.

The input data for a Coulomb solver consists of only two entities; the charge $q_i$ of a particle $i$ and the position $\mathbf{x_i}$ in $\mathscr{R^3}$ with the components $x_i$ , $y_i$ , and $z_i$ . The position of the particle $i$ at $\mathbf{x_i}$ is defined with its components as
```math
\mathbf{x_i} = \begin{pmatrix} x_i \\ y_i \\ z_i \end{pmatrix}
```

The distance between two particles $|\mathbf{x_i} − \mathbf{x_j}|$ can be computed as
```math
|\mathbf{x_i} − \mathbf{x_j}| = \sqrt{(x_i - x_j)^2 + (y_i - y_j)^2 + (z_i - z_j)^2}
```

Physical properties relevant for the simulation are the Coulomb potential $\Phi$ and the Coulomb force $\mathbf{F}$ as well as the Coulomb energy $\mathit{E}_c$ of the system. A Coulomb solver should at least calculate the forces acting on all particles due to the presence of the other particles. Therefore, one has to compute the Coulomb forces $\mathbf{F}_i$ with $i = 1 ... N$ acting on each particle via
```math
\mathbf{F}_i = q_i \sum_{j = 1}^{N} q_j \dfrac{\mathbf{x_i} - \mathbf{x_j}}{|\mathbf{x_i} − \mathbf{x_j}|^3}, (i \ne j)
```

### EXTRA POINTS I
Some MD codes also need the potential $\Phi$ at each particle position. A compile-time switch (e.g. template parameter) should be used to enable or disable this feature. Compute the Coulomb potentials $\Phi_i$ with $i = 1 ... N$ at each particle position $\mathbf{x_i}$ due to all other charges via
```math
\Phi_i = \sum_{j = 1}^{N} \dfrac{q_j}{|\mathbf{x_i} − \mathbf{x_j}|}, (i \ne j)
```

### EXTRA POINTS II
Another physical property – the Coulomb energy – might be useful for some applications. Compute the Coulomb energy $\mathit{E}_c$ of the system represented by
```math
\mathit{E}_c = \dfrac{1}{2} \sum_{i = 1}^{N} \sum_{j = 1}^{N} \dfrac{q_i q_j}{|\mathbf{x_i} − \mathbf{x_j}|}, (i \ne j)
```

## 1 Requirements
### 1.1 General Requirements
Correct results for all entities up to datatype precision are mandatory. Additional numerical errors (round-off) arising through a different summation order can be neglected in this exercise. The goal of the exercise is to utilize C++ features (e.g. STL) in order to write clean, structured code, that also exhibits strong performance on modern computer architectures. You are free to use other third-party libraries to harvest most of the available performance.

### 1.2 Further Optional Features
Additional goodies could be:
* Build environment via cmake (www.cmake.org).
* Testing via Catch2 (github.com/catchorg/Catch2).

### 1.3 Format of Input/Output
The input and output files are defined as simple text files. The input file consists of four columns for each particle. The first column contains the charge $q_i$ , the second, third and fourth columns hold the Cartesian coordinates $x_i$, $y_i$, and $z_i$ of the particle $i$. The output file must contain the force components $F_{x_i}$, $F_{y_i}$, and $F_{z_i}$ of $\mathbf{F}_i$ in the first, second and third column. The potential $\Phi_i$ should be stored in the fourth column if computed. For the Coulomb energy $\mathit{E}_c$ it is sufficient to print it together with other outputs (timings, additional information) on the console.

Input data [6MB] https://fz-juelich.sciebo.de/s/xwGTTCypRIwhljU

## 2 Guidelines
The implementation should allow different data types for the input and output data, e.g. single precision (32 bit) or double precision (64 bit). The type should be a compile-time parameter. Avoid excessive use of preprocessor macros, use C++ type safety features whenever possible. Do not use third-party libraries containing the Coulomb solver for this exercise. Also getting support from ChatGPT or similar AI tools is unwelcome.

### 2.1 Workflow
First, read the data from the input file. Then, start the clock and measure the computation time for the forces, potentials and Coulomb energy. Stop the time measurement. Finally write the results to the output file. Print the measured computation time on the console.

### 2.2 Language & Compiler Version
The Coulomb solver should be written in C++. The source code should also contain appropriate comments in English. The program should compile with all modern C++ compilers. However, if a performance gain can be achieved on the presented hardware make sure one of the following compilers can be used: GNU C++ compiler up to version 11.3 or Clang compiler up to version 13.0.1. C++20 features (e.g. ranges, pipe operator, constexpr, concepts) can be utilized as well.

## 3 Discussion
Some questions that may arise are:
* What is the computational complexity of the solved problem?
* How long does the computation of one time step take on your machine?
* How long would the computation of 3 trillion particles take?
* How many digits do you expect to be correct for such an computation?
* What is the limiting performance factor (bandwidth, clock speed, etc.)?
* What language features are important for the performance?
* Why did you chose one language feature/design pattern over another?
