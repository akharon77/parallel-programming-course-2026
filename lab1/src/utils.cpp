#include "utils.h"

uint64_t ComputePercentile(uint64_t count,
                           const std::array<uint64_t, kNumBuckets> &buckets,
                           double percentile) {
  if (count == 0)
    return 0;

  uint64_t threshold = count * percentile;
  uint64_t accumulated = 0;
  for (size_t i = 0; i < kNumBuckets; ++i) {
    accumulated += buckets[i];

    if (accumulated >= threshold) {
      return i * kBucketSpanMs;
    }
  }

  return 255 * kBucketSpanMs;
}
