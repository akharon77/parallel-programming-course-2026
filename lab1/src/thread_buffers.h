#pragma once

#include "metrics_collector.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <limits>

struct Buf {
  std::array<uint64_t, kNumBuckets> buckets{};
  uint64_t count = 0;
  uint64_t sum = 0;
  uint64_t min = std::numeric_limits<uint64_t>::max();
  uint64_t max = 0;

  void clear() {
    buckets.fill(0);
    count = 0;
    sum = 0;
    min = std::numeric_limits<uint64_t>::max();
    max = 0;
  }
};

//
// Not enum class because I don't want to do static_cast.
//
enum BufferSelectFlag : int { kNowhere = -1, kBuffer0 = 0, kBuffer1 = 1 };

static constexpr size_t kBufCount = 2;

struct alignas(64) ThreadBuffers {
  std::atomic<int> inside{BufferSelectFlag::kNowhere};
  Buf buf[kBufCount];
};
