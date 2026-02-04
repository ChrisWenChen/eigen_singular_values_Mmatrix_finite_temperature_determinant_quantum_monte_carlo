#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
  (void)argv;
  MPI_Init(&argc, &argv);

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    std::cout << "bench_dense_operator (placeholder)" << std::endl;
  }

  MPI_Finalize();
  return 0;
}
