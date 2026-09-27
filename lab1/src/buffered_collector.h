#pragma once

#include "metrics_collector.h"
#include "thread_buffers.h"
#include "thread_local_collector.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

class BufferedCollector : public MetricsCollector {
public:
  BufferedCollector() : id_(next_collector_id()) {}

  void Record(uint64_t value) override;
  Snapshot GetSnapshot() override;

private:
  const uint64_t id_;
  std::mutex snap_mutex_;
  Buf global_summary_;
  std::atomic<BufferSelectFlag> active_{BufferSelectFlag::kBuffer0};
  std::vector<std::unique_ptr<ThreadBuffers>> all_states_;

  ThreadBuffers *GetMyBuffers();
};