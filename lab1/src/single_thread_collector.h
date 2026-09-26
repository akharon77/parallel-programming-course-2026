#pragma once

#include "metrics_collector.h"

#include <limits>

class SingleThreadCollector : public MetricsCollector {
public:
  void Record(uint64_t value) override;
  Snapshot GetSnapshot() override;

private:
  std::array<uint64_t, kNumBuckets> buckets_{};
  uint64_t count_ = 0;
  uint64_t sum_ = 0;
  uint64_t min_ = std::numeric_limits<uint64_t>::max();
  uint64_t max_ = 0;
};
