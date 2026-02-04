#pragma once

#include <string>

namespace core {

enum class HSType {
  HIRSCH_SPIN = 0,
  SU2_DENSITY = 1
};

struct ModelParams {
  int Lx = 4;
  int Ly = 4;
  int Lz = 1;
  int L = 10;
  double dtau = 0.1;
  double U = 4.0;
  double mu = 0.0;
  double t = 1.0;
  bool use_tprime = false;
  double tprime = 0.0;
  HSType hs_type = HSType::HIRSCH_SPIN;

  static ModelParams load_or_default(const std::string& path = "input.toml");
  static ModelParams load_or_default_json(const std::string& path = "input.json");
};

const char* hs_type_name(HSType type);

}  // namespace core
