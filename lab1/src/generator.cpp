#include <cassert>
#include <cmath>
#include <random>

#include "generator.h"

static constexpr uint32_t kZipfRandomSeed = 0xCAFE;
static constexpr double kZipfExponent = 1.15;
static constexpr double kEps = 1e-6;

static double ZipfDense(uint64_t k) { return 1. / std::pow(k, kZipfExponent); }

static std::vector<double> GenerateZipfDistFun(uint64_t max_val) {
  std::vector<double> dist_fun(max_val + 1, 0.0);
  double sum = 0.0;
  for (int k = 1; k < max_val + 1; ++k) {
    sum += ZipfDense(k);
    dist_fun[k] = sum;
  }

  for (int k = 1; k < max_val + 1; ++k) {
    //
    // Monotonic from 0 to 1.
    //
    dist_fun[k] /= sum;
    assert(dist_fun[k] > -kEps && dist_fun[k] < 1. + kEps);
    assert(dist_fun[k] > dist_fun[k - 1] + kEps);
  }

  return dist_fun;
}

std::vector<uint64_t> GenerateZipfValues(uint64_t max_val, size_t size) {
  std::vector<uint64_t> values(size);
  std::mt19937 gen(kZipfRandomSeed);

  auto dist_fun = GenerateZipfDistFun(max_val);

  std::uniform_real_distribution<double> dist(0.0, 1.0);
  for (size_t i = 0; i < size; ++i) {
    //
    // 1. Gen Y uniform 0 to 1,
    // 2. Map to chart,
    // 3. Get X.
    //
    double p = dist(gen);
    auto it = std::lower_bound(dist_fun.begin(), dist_fun.end(), p);
    values[i] = std::distance(dist_fun.begin(), it);
  }

  return values;
}
