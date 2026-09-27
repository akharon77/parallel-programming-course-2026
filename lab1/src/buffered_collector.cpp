#include "buffered_collector.h"
#include "thread_buffers.h"
#include "utils.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

void BufferedCollector::Record(uint64_t value) {
  ThreadBuffers *my_buffers = GetMyBuffers();
  BufferSelectFlag active_buf;

  while (true) {
    active_buf = active_.load();
    my_buffers->inside.store(active_buf);

    if (active_.load() == active_buf) {
      break;
    }

    my_buffers->inside.store(BufferSelectFlag::kNowhere);
  }

  assert(active_buf != BufferSelectFlag::kNowhere);
  uint64_t bucket = std::min(value / kBucketSpanMs, kNumBuckets - 1);

  my_buffers->buf[active_buf].buckets[bucket]++;
  my_buffers->buf[active_buf].count++;
  my_buffers->buf[active_buf].sum += value;

  if (value < my_buffers->buf[active_buf].min)
    my_buffers->buf[active_buf].min = value;

  if (value > my_buffers->buf[active_buf].max)
    my_buffers->buf[active_buf].max = value;

  my_buffers->inside.store(BufferSelectFlag::kNowhere,
                           std::memory_order_release);
}

Snapshot BufferedCollector::GetSnapshot() {
  std::lock_guard<std::mutex> guard(snap_mutex_);

  BufferSelectFlag old_active = active_.load();
  BufferSelectFlag new_active = (old_active == BufferSelectFlag::kBuffer0)
                                    ? BufferSelectFlag::kBuffer1
                                    : BufferSelectFlag::kBuffer0;
  active_.store(new_active);

  for (const auto &state : all_states_) {
    while (state->inside.load() == old_active) {
      std::this_thread::yield();
    }

    for (size_t i = 0; i < kNumBuckets; ++i) {
      global_summary_.buckets[i] += state->buf[old_active].buckets[i];
    }

    global_summary_.count += state->buf[old_active].count;
    global_summary_.sum += state->buf[old_active].sum;
    global_summary_.min =
        std::min(global_summary_.min, state->buf[old_active].min);
    global_summary_.max =
        std::max(global_summary_.max, state->buf[old_active].max);

    state->buf[old_active].clear();
  }

  return {
      global_summary_.buckets,
      global_summary_.count,
      global_summary_.sum,
      global_summary_.min,
      global_summary_.max,
      ComputePercentile(global_summary_.count, global_summary_.buckets, p50),
      ComputePercentile(global_summary_.count, global_summary_.buckets, p99)};
}

ThreadBuffers *BufferedCollector::GetMyBuffers() {
  struct TlsSlot {
    uint64_t id = 0;
    ThreadBuffers *state = nullptr;
  };
  static thread_local TlsSlot slot;

  if (slot.id != id_) {
    auto s = std::make_unique<ThreadBuffers>();
    ThreadBuffers *raw = s.get();
    {
      std::lock_guard<std::mutex> g(snap_mutex_);
      all_states_.push_back(std::move(s));
    }
    slot.id = id_;
    slot.state = raw;
  }
  return slot.state;
}
