#include "core/hs_field.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

namespace core {

HSField::HSField(int L, int n_sites, std::vector<int8_t> data)
  : L_(L), n_sites_(n_sites), data_(std::move(data)) {}

namespace {

bool load_binary(const std::string& path, int* L, int* n_sites, std::vector<int8_t>* data) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;

  int32_t L32 = 0;
  int32_t N32 = 0;
  in.read(reinterpret_cast<char*>(&L32), sizeof(int32_t));
  in.read(reinterpret_cast<char*>(&N32), sizeof(int32_t));
  if (!in) return false;

  if (L32 <= 0 || N32 <= 0) return false;

  std::vector<int8_t> buffer(static_cast<size_t>(L32) * static_cast<size_t>(N32));
  in.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
  if (!in) return false;

  *L = static_cast<int>(L32);
  *n_sites = static_cast<int>(N32);
  *data = std::move(buffer);
  return true;
}

std::vector<int8_t> generate_field(int L, int n_sites) {
  const size_t total = static_cast<size_t>(L) * static_cast<size_t>(n_sites);
  std::vector<int8_t> data(total);
  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> dist(0, 1);
  for (size_t i = 0; i < total; ++i) {
    data[i] = dist(rng) ? 1 : -1;
  }
  return data;
}

}  // namespace

HSField HSField::load_or_generate(int L, int n_sites, const std::string& path) {
  int file_L = 0;
  int file_N = 0;
  std::vector<int8_t> data;
  if (load_binary(path, &file_L, &file_N, &data)) {
    if (file_L != L || file_N != n_sites) {
      std::cerr << "Warning: hsfield.bin dimension mismatch, regenerating." << std::endl;
      data = generate_field(L, n_sites);
      return HSField(L, n_sites, std::move(data));
    }
    return HSField(file_L, file_N, std::move(data));
  }

  std::cerr << "Warning: hsfield.bin not found, generating deterministic HS field." << std::endl;
  data = generate_field(L, n_sites);
  return HSField(L, n_sites, std::move(data));
}

}  // namespace core
