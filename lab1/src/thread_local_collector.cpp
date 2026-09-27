#include "thread_local_collector.h"
#include "metrics_collector.h"
#include "utils.h"

void relaxed_add(std::atomic<uint64_t> &c, uint64_t delta) {
  c.store(c.load(std::memory_order_relaxed) + delta, std::memory_order_relaxed);
}

ThreadState *ThreadLocalCollector::get_my_state() {
  struct TLSSlot {
    uint64_t id = 0;
    ThreadState *state = nullptr;
  };

  static thread_local TLSSlot slot;
  if (slot.id != id_) {
    auto state = std::make_unique<ThreadState>();
    ThreadState *raw = state.get();

    {
      std::lock_guard<std::mutex> g(list_lock_);
      all_states_.push_back(std::move(state));
    }

    slot.id = id_;
    slot.state = raw;
  }
  return slot.state;
}

void ThreadLocalCollector::Record(uint64_t value) {
  ThreadState *state = get_my_state();
  uint64_t b = std::min(value / kBucketSpanMs, kNumBuckets - 1);

  relaxed_add(state->buckets[b], 1);
  relaxed_add(state->count, 1);
  relaxed_add(state->sum, value);

  if (value < state->min.load(std::memory_order_relaxed))
    state->min.store(value, std::memory_order_relaxed);
  if (value > state->max.load(std::memory_order_relaxed))
    state->max.store(value, std::memory_order_relaxed);
}

Snapshot ThreadLocalCollector::GetSnapshot() {
  std::array<uint64_t, kNumBuckets> buckets{};
  uint64_t count = 0;
  uint64_t sum = 0;
  uint64_t min = std::numeric_limits<uint64_t>::max();
  uint64_t max = 0;

  {
    std::lock_guard<std::mutex> g(list_lock_);

    for (const auto &state : all_states_) {
      for (size_t i = 0; i < kNumBuckets; ++i) {
        buckets[i] += state->buckets[i].load(std::memory_order_relaxed);
      }

      count += state->count.load(std::memory_order_relaxed);
      sum += state->sum.load(std::memory_order_relaxed);

      uint64_t s_min = state->min.load(std::memory_order_relaxed);
      if (s_min < min)
        min = s_min;

      uint64_t s_max = state->max.load(std::memory_order_relaxed);
      if (s_max > max)
        max = s_max;
    }
  }

  return {buckets,
          count,
          sum,
          min,
          max,
          ComputePercentile(count, buckets, p50),
          ComputePercentile(count, buckets, p99)};
}
