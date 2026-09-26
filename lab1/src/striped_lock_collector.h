#pragma once

#include <atomic>
#include <mutex>

#include "metrics_collector.h"

static constexpr size_t kNumShards = 16;

class StripedLockCollector : public MetricsCollector {
public:
  void Record(uint64_t value) override;
  Snapshot GetSnapshot() override;

private:
  std::array<std::mutex, kNumShards> locks_;
  std::array<uint64_t, kNumBuckets> buckets_{};
  std::atomic<uint64_t> count_{0};
  std::atomic<uint64_t> sum_{0};
  std::atomic<uint64_t> min_{std::numeric_limits<uint64_t>::max()};
  std::atomic<uint64_t> max_{0};
};