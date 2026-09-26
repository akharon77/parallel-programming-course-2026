#include "global_lock_collector.h"

void GlobalLockCollector::Record(uint64_t value) {
  std::lock_guard<std::mutex> lock(mutex_);
  SingleThreadCollector::Record(value);
}

Snapshot GlobalLockCollector::GetSnapshot() {
  std::lock_guard<std::mutex> lock(mutex_);
  return SingleThreadCollector::GetSnapshot();
}
