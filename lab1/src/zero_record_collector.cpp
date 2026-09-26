#include "zero_record_collector.h"

void ZeroRecordCollector::Record(uint64_t value) {
  std::lock_guard<std::mutex> lock(mutex_);
}

Snapshot ZeroRecordCollector::GetSnapshot() {
  std::lock_guard<std::mutex> lock(mutex_);
  return SingleThreadCollector::GetSnapshot();
}
