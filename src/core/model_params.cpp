#include "core/model_params.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace core {

namespace {

std::string trim(const std::string& s) {
  size_t b = 0;
  while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
  size_t e = s.size();
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
  return s.substr(b, e - b);
}

std::string strip_quotes(const std::string& s) {
  if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

bool parse_bool(const std::string& s, bool* out) {
  std::string v;
  v.reserve(s.size());
  for (char c : s) v.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  if (v == "true" || v == "1") { *out = true; return true; }
  if (v == "false" || v == "0") { *out = false; return true; }
  return false;
}

bool parse_int(const std::string& s, int* out) {
  try {
    size_t pos = 0;
    int v = std::stoi(s, &pos);
    if (pos != s.size()) return false;
    *out = v;
    return true;
  } catch (...) {
    return false;
  }
}

bool parse_double(const std::string& s, double* out) {
  try {
    size_t pos = 0;
    double v = std::stod(s, &pos);
    if (pos != s.size()) return false;
    *out = v;
    return true;
  } catch (...) {
    return false;
  }
}

HSType parse_hs_type(const std::string& s, bool* ok) {
  std::string v;
  v.reserve(s.size());
  for (char c : s) v.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
  if (v == "HIRSCH_SPIN") { *ok = true; return HSType::HIRSCH_SPIN; }
  if (v == "SU2_DENSITY") { *ok = true; return HSType::SU2_DENSITY; }
  *ok = false;
  return HSType::HIRSCH_SPIN;
}

void apply_kv(ModelParams* p, const std::string& key, const std::string& value) {
  if (key == "Lx") { int v; if (parse_int(value, &v)) p->Lx = v; return; }
  if (key == "Ly") { int v; if (parse_int(value, &v)) p->Ly = v; return; }
  if (key == "Lz") { int v; if (parse_int(value, &v)) p->Lz = v; return; }
  if (key == "L") { int v; if (parse_int(value, &v)) p->L = v; return; }
  if (key == "dtau") { double v; if (parse_double(value, &v)) p->dtau = v; return; }
  if (key == "U") { double v; if (parse_double(value, &v)) p->U = v; return; }
  if (key == "mu") { double v; if (parse_double(value, &v)) p->mu = v; return; }
  if (key == "t") { double v; if (parse_double(value, &v)) p->t = v; return; }
  if (key == "tprime") { double v; if (parse_double(value, &v)) p->tprime = v; return; }
  if (key == "use_tprime") { bool v; if (parse_bool(value, &v)) p->use_tprime = v; return; }
  if (key == "hs_type") {
    bool ok = false;
    HSType t = parse_hs_type(strip_quotes(value), &ok);
    if (ok) p->hs_type = t;
    return;
  }
}

ModelParams parse_key_value_file(const std::string& path, bool* ok) {
  ModelParams p;
  *ok = false;

  std::ifstream in(path);
  if (!in) return p;

  std::string line;
  while (std::getline(in, line)) {
    std::string s = trim(line);
    if (s.empty()) continue;
    if (s[0] == '#') continue;
    size_t eq = s.find('=');
    size_t colon = s.find(':');
    size_t sep = std::string::npos;
    if (eq != std::string::npos) sep = eq;
    if (colon != std::string::npos) sep = std::min(sep, colon);
    if (sep == std::string::npos) continue;

    std::string key = trim(s.substr(0, sep));
    std::string val = trim(s.substr(sep + 1));
    if (!val.empty() && val.back() == ',') val.pop_back();
    key = strip_quotes(key);
    val = strip_quotes(val);
    apply_kv(&p, key, val);
  }

  *ok = true;
  return p;
}

}  // namespace

const char* hs_type_name(HSType type) {
  switch (type) {
    case HSType::HIRSCH_SPIN: return "HIRSCH_SPIN";
    case HSType::SU2_DENSITY: return "SU2_DENSITY";
    default: return "UNKNOWN";
  }
}

ModelParams ModelParams::load_or_default(const std::string& path) {
  bool ok = false;
  ModelParams p;

  if (!path.empty() && std::filesystem::exists(path)) {
    p = parse_key_value_file(path, &ok);
    if (ok) return p;
  }

  const std::string json_path = "input.json";
  if (std::filesystem::exists(json_path)) {
    p = parse_key_value_file(json_path, &ok);
    if (ok) return p;
  }

  std::cerr << "Warning: input.toml/input.json not found, using built-in defaults." << std::endl;
  return ModelParams{};
}

ModelParams ModelParams::load_or_default_json(const std::string& path) {
  bool ok = false;
  ModelParams p = parse_key_value_file(path, &ok);
  if (ok) return p;

  std::cerr << "Warning: input.json not found, using built-in defaults." << std::endl;
  return ModelParams{};
}

}  // namespace core
