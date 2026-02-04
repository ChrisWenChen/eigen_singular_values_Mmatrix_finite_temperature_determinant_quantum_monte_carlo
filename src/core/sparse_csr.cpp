#include "core/sparse_csr.hpp"

#include <stdexcept>

namespace core {

void SparseCSR::apply(const std::vector<double>& x, std::vector<double>* y) const {
  if (static_cast<int>(x.size()) != ncols) {
    throw std::runtime_error("SparseCSR::apply dimension mismatch");
  }
  y->assign(static_cast<size_t>(nrows), 0.0);
  for (int i = 0; i < nrows; ++i) {
    const int start = row_ptr[i];
    const int end = row_ptr[i + 1];
    double sum = 0.0;
    for (int idx = start; idx < end; ++idx) {
      sum += values[idx] * x[col_idx[idx]];
    }
    (*y)[i] = sum;
  }
}

}  // namespace core
