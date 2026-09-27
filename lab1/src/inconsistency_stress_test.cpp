#include "inconsistency_stress_test.h"

#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

static constexpr size_t kNumWriters = 4;
static constexpr size_t kNumSnapshots = 10000;
static constexpr size_t kStartCoeff = 1000;

void RunInconsistencyStressTest(MetricsCollector *collector,
                                const std::vector<uint64_t> &values) {
  std::atomic<bool> stop_flag{false};
  std::vector<std::thread> writers;
  std::array<size_t, kNumWriters> count_for_writer;

  for (size_t k = 0; k < kNumWriters; ++k) {
    writers.emplace_back([&, k]() {
      size_t local_count = 0;
      size_t i = (k * kStartCoeff) % values.size();

      while (!stop_flag.load()) {
        collector->Record(values[i]);
        ++local_count;

        ++i;
        if (i == values.size()) {
          i = 0;
        }
      }

      count_for_writer[k] = local_count;
    });
  }

  size_t broken_snapshots = 0;
  size_t sum_less_than_count = 0;
  size_t sum_greater_than_count = 0;

  for (size_t i = 0; i < kNumSnapshots; ++i) {
    Snapshot snapshot = collector->GetSnapshot();

    uint64_t sum_buckets = 0;
    for (uint64_t b : snapshot.buckets) {
      sum_buckets += b;
    }

    if (sum_buckets != snapshot.count) {
      broken_snapshots++;

      if (sum_buckets < snapshot.count) {
        sum_less_than_count++;
      } else {
        assert(sum_buckets > snapshot.count);
        sum_greater_than_count++;
      }
    }
  }

  stop_flag.store(true);
  for (size_t k = 0; k < kNumWriters; ++k) {
    writers[k].join();
  }

  size_t exact_count = 0;
  for (size_t k = 0; k < kNumWriters; ++k) {
    exact_count += count_for_writer[k];
  }

  Snapshot final_snapshot = collector->GetSnapshot();

  double broken_percent =
      (static_cast<double>(broken_snapshots) / kNumSnapshots) * 100.0;

  std::cout << "Inconsistency stress test:" << std::endl;
  std::cout << "  Broken snapshots: " << broken_percent << "%" << std::endl;
  std::cout << "    Sum < count: " << sum_less_than_count << std::endl;
  std::cout << "    Sum > count: " << sum_greater_than_count << std::endl;
  std::cout << "  Exact count of record(): " << exact_count << std::endl;
  std::cout << "  Count from final snapshot: " << final_snapshot.count
            << std::endl;
}
