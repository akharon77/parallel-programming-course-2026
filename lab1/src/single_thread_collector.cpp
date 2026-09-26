#include "single_thread_collector.h"
#include "utils.h"

#include <array>

void SingleThreadCollector::Record(uint64_t value) {
  uint64_t bucket_index = std::min(value / kBucketSpanMs, 255ul);

  ++buckets_[bucket_index];
  ++count_;
  sum_ += value;

  if (value < min_)
    min_ = value;

  if (value > max_)
    max_ = value;
}

Snapshot SingleThreadCollector::GetSnapshot() {
  return {buckets_,
          count_,
          sum_,
          min_,
          max_,
          ComputePercentile(count_, buckets_, p50),
          ComputePercentile(count_, buckets_, p99)};
}
