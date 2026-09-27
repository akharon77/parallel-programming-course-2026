#pragma once

#include <array>
#include <atomic>
#include <cstdint>

struct ThreadState {
  std::array<std::atomic<uint64_t>, 256> buckets{};
  std::atomic<uint64_t> count{0};
  std::atomic<uint64_t> sum{0};
  std::atomic<uint64_t> min{UINT64_MAX};
  std::atomic<uint64_t> max{0};
};