#include "core/hs_field.hpp"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <iostream>

int main() {
  const int L = 4;
  const int N = 9;
  const std::string path = "hsfield_test.bin";

  std::filesystem::remove(path);

  core::HSField f1 = core::HSField::load_or_generate(L, N, path);
  core::HSField f2 = core::HSField::load_or_generate(L, N, path);

  assert(f1.L() == L);
  assert(f1.n_sites() == N);
  assert(f2.L() == L);
  assert(f2.n_sites() == N);

  const auto& d1 = f1.data();
  const auto& d2 = f2.data();
  assert(d1.size() == d2.size());
  for (size_t i = 0; i < d1.size(); ++i) {
    assert(d1[i] == d2[i]);
  }

  std::filesystem::remove(path);
  std::cout << "HSField reproducibility test passed." << std::endl;
  return 0;
}
