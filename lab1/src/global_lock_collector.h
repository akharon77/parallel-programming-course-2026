#pragma once

#include "single_thread_collector.h"

#include <mutex>

class GlobalLockCollector : public SingleThreadCollector {
public:
  void Record(uint64_t value) override;
  Snapshot GetSnapshot() override;

private:
  std::mutex mutex_;
};
