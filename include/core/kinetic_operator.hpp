#pragma once

#include "core/model_params.hpp"
#include "core/sparse_csr.hpp"

#include <vector>

namespace core {

class KineticOperator {
 public:
  explicit KineticOperator(const ModelParams& params);

  int n_sites() const { return n_sites_; }
  const SparseCSR& K() const { return K_; }

  void apply_K(const std::vector<double>& x, std::vector<double>* y) const;

 private:
  int Lx_ = 0;
  int Ly_ = 0;
  int Lz_ = 0;
  int n_sites_ = 0;
  double t_ = 0.0;
  bool use_tprime_ = false;
  double tprime_ = 0.0;
  SparseCSR K_;

  int index(int x, int y, int z) const;
  void build_matrix();
};

}  // namespace core
