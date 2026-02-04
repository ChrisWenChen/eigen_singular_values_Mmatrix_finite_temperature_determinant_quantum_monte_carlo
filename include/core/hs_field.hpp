#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core {

class HSField {
 public:
  HSField() = default;
  HSField(int L, int n_sites, std::vector<int8_t> data);

  static HSField load_or_generate(int L, int n_sites, const std::string& path = "hsfield.bin");

  int L() const { return L_; }
  int n_sites() const { return n_sites_; }
  const std::vector<int8_t>& data() const { return data_; }

 private:
  int L_ = 0;
  int n_sites_ = 0;
  std::vector<int8_t> data_;
};

}  // namespace core
