#include <iostream>

extern "C" {
void dgemm_(const char* transa, const char* transb, const int* m, const int* n,
           const int* k, const double* alpha, const double* a, const int* lda,
           const double* b, const int* ldb, const double* beta, double* c,
           const int* ldc);
}

int main() {
  std::cout << "BLAS/LAPACK backend: " << LINALG_BACKEND_NAME << std::endl;

  // 2x2 matrix multiply: C = A * B
  // A = [1 2; 3 4], B = [5 6; 7 8]
  // Expected C = [19 22; 43 50]
  double A[4] = {1, 3, 2, 4};
  double B[4] = {5, 7, 6, 8};
  double C[4] = {0, 0, 0, 0};

  const char trans = 'N';
  const int m = 2, n = 2, k = 2;
  const int lda = 2, ldb = 2, ldc = 2;
  const double alpha = 1.0;
  const double beta = 0.0;

  dgemm_(&trans, &trans, &m, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

  const double expected[4] = {19, 43, 22, 50};
  bool ok = true;
  for (int i = 0; i < 4; ++i) {
    double diff = C[i] - expected[i];
    if (diff < 0) diff = -diff;
    if (diff > 1e-12) {
      ok = false;
      break;
    }
  }

  if (ok) {
    std::cout << "GEMM check: OK" << std::endl;
    return 0;
  }

  std::cout << "GEMM check: FAILED" << std::endl;
  std::cout << "Computed C (column-major): [" << C[0] << " " << C[2]
            << "; " << C[1] << " " << C[3] << "]" << std::endl;
  return 1;
}
