#include "core/kinetic_operator.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace core {

KineticOperator::KineticOperator(const ModelParams& params)
  : Lx_(params.Lx), Ly_(params.Ly), Lz_(params.Lz),
    n_sites_(params.Lx * params.Ly * params.Lz),
    t_(params.t), use_tprime_(params.use_tprime), tprime_(params.tprime) {
  if (Lx_ <= 0 || Ly_ <= 0 || Lz_ <= 0) {
    throw std::runtime_error("Invalid lattice dimensions");
  }
  build_matrix();
}

int KineticOperator::index(int x, int y, int z) const {
  return (z * Ly_ + y) * Lx_ + x;
}

void KineticOperator::build_matrix() {
  std::vector<std::vector<std::pair<int, double>>> rows(static_cast<size_t>(n_sites_));

  for (int z = 0; z < Lz_; ++z) {
    for (int y = 0; y < Ly_; ++y) {
      for (int x = 0; x < Lx_; ++x) {
        const int i = index(x, y, z);

        const int xp = (x + 1) % Lx_;
        const int xm = (x - 1 + Lx_) % Lx_;
        const int yp = (y + 1) % Ly_;
        const int ym = (y - 1 + Ly_) % Ly_;
        const int zp = (z + 1) % Lz_;
        const int zm = (z - 1 + Lz_) % Lz_;

        rows[i].push_back({index(xp, y, z), -t_});
        rows[i].push_back({index(xm, y, z), -t_});
        rows[i].push_back({index(x, yp, z), -t_});
        rows[i].push_back({index(x, ym, z), -t_});

        if (Lz_ > 1) {
          rows[i].push_back({index(x, y, zp), -t_});
          rows[i].push_back({index(x, y, zm), -t_});
        }

        if (use_tprime_ && Lz_ == 1) {
          rows[i].push_back({index(xp, yp, z), -tprime_});
          rows[i].push_back({index(xp, ym, z), -tprime_});
          rows[i].push_back({index(xm, yp, z), -tprime_});
          rows[i].push_back({index(xm, ym, z), -tprime_});
        }
      }
    }
  }

  K_.nrows = n_sites_;
  K_.ncols = n_sites_;
  K_.row_ptr.assign(static_cast<size_t>(n_sites_) + 1, 0);
  for (int i = 0; i < n_sites_; ++i) {
    K_.row_ptr[i + 1] = K_.row_ptr[i] + static_cast<int>(rows[i].size());
  }
  const int nnz = K_.row_ptr[n_sites_];
  K_.col_idx.resize(static_cast<size_t>(nnz));
  K_.values.resize(static_cast<size_t>(nnz));

  int cursor = 0;
  for (int i = 0; i < n_sites_; ++i) {
    for (const auto& kv : rows[i]) {
      K_.col_idx[cursor] = kv.first;
      K_.values[cursor] = kv.second;
      ++cursor;
    }
  }
}

void KineticOperator::apply_K(const std::vector<double>& x, std::vector<double>* y) const {
  K_.apply(x, y);
}

}  // namespace core
