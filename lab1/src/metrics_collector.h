#pragma once

#include <cstdlib>
#include <cstdint>

constexpr size_t kNumBuckets = 256;
constexpr uint64_t kBucketSpanMs = 4;

struct Snapshot {
  uint64_t buckets[kNumBuckets];
  uint64_t count;
  uint64_t sum;
  uint64_t min;
  uint64_t max;
  uint64_t p50;
  uint64_t p99;
};

class MetricsCollector {
public:
  virtual ~MetricsCollector() = default;
  virtual void Record(uint64_t value) = 0;
  virtual Snapshot GetSnapshot() = 0;
};
