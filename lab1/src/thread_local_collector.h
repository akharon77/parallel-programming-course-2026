#pragma once

#include "metrics_collector.h"
#include "thread_state.h"

#include <memory>
#include <mutex>
#include <vector>

inline uint64_t next_collector_id() {
  static std::atomic<uint64_t> counter{1};
  return counter.fetch_add(1);
}

class ThreadLocalCollector : public MetricsCollector {
public:
  void Record(uint64_t value) override;
  Snapshot GetSnapshot() override;

private:
  ThreadState *get_my_state();

private:
  const uint64_t id_ = next_collector_id();
  std::mutex list_lock_;
  std::vector<std::unique_ptr<ThreadState>> all_states_;
};
