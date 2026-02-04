#pragma once

#include <vector>

namespace core {

struct SparseCSR {
  int nrows = 0;
  int ncols = 0;
  std::vector<int> row_ptr;
  std::vector<int> col_idx;
  std::vector<double> values;

  void apply(const std::vector<double>& x, std::vector<double>* y) const;
};

}  // namespace core
