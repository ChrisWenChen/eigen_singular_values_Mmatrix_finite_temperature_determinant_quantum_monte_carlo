#include <petsc.h>
#include <iostream>

int main(int argc, char** argv) {
  PetscErrorCode ierr = PetscInitialize(&argc, &argv, nullptr, nullptr);
  if (ierr) {
    return static_cast<int>(ierr);
  }

  int rank = 0;
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
  if (rank == 0) {
    std::cout << "bench_petsc_shell (placeholder)" << std::endl;
  }

  PetscFinalize();
  return 0;
}
