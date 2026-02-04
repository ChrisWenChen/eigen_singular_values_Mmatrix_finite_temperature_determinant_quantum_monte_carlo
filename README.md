# eigen_singular_values_Mmatrix_finite_temperature_determinant_quantum_monte_carlo

C++17 project scaffold for benchmarking singular values of the determinant QMC M-matrix across multiple backends. This repository is designed to be cross-platform (Linux/macOS) and MPI-enabled, with optional PETSc/SLEPc integration.

## Build Requirements

- CMake >= 3.16
- C++17 compiler (clang or gcc)
- MPI (OpenMPI, MPICH, or system MPI)

Optional (for PETSc/SLEPc backends):
- PETSc
- SLEPc

## Build (Linux/macOS)

### Non-PETSc (default)

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

This builds:
- `bench_dense_matrix`
- `bench_dense_operator`

### With PETSc/SLEPc

```bash
mkdir -p build
cd build
cmake .. -DENABLE_PETSC=ON
cmake --build . -j
```

This additionally builds:
- `bench_petsc_matrix`
- `bench_petsc_shell`

If PETSc/SLEPc are installed in non-default locations, set the usual CMake hints, for example:

```bash
cmake .. -DENABLE_PETSC=ON -DPETSC_DIR=/path/to/petsc -DSLEPC_DIR=/path/to/slepc
```

## Run

These are placeholders and will be extended in subsequent steps. MPI is initialized in all executables.

Examples:

```bash
mpirun -n 2 ./bench_dense_matrix
mpirun -n 2 ./bench_dense_operator
```

For PETSc executables, only PETSc/SLEPc command-line options are accepted.

```bash
mpirun -n 2 ./bench_petsc_matrix -log_view
mpirun -n 2 ./bench_petsc_shell -log_view
```
