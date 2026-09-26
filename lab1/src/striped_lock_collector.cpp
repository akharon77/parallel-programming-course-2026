#include "striped_lock_collector.h"
#include "utils.h"

void StripedLockCollector::Record(uint64_t value) {
  uint64_t bucket_num = std::min(value / kBucketSpanMs, kNumBuckets - 1);

  {
    size_t shard_num = bucket_num % kNumShards;
    std::lock_guard<std::mutex> lock(locks_[shard_num]);
    buckets_[bucket_num]++;
  }

  count_.fetch_add(1);
  sum_.fetch_add(value);

  uint64_t curr_min = min_.load();
  while (value < curr_min && !min_.compare_exchange_weak(curr_min, value)) {
  }

  uint64_t curr_max = max_.load();
  while (value > curr_max && !max_.compare_exchange_weak(curr_max, value)) {
  }
}

Snapshot StripedLockCollector::GetSnapshot() {
  std::array<uint64_t, kNumBuckets> local_buckets{};

  for (size_t shard_num = 0; shard_num < kNumShards; ++shard_num) {
    std::lock_guard<std::mutex> lock(locks_[shard_num]);

    for (size_t bucket_num = shard_num; bucket_num < kNumBuckets;
         bucket_num += kNumShards) {
      local_buckets[bucket_num] = buckets_[bucket_num];
    }
  }

  uint64_t local_count = count_.load();
  uint64_t sum = sum_.load();
  uint64_t min = min_.load();
  uint64_t max = max_.load();

  return {local_buckets,
          local_count,
          sum,
          min,
          max,
          ComputePercentile(local_count, local_buckets, p50),
          ComputePercentile(local_count, local_buckets, p99)};
}
