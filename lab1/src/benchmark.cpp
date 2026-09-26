#include "benchmark.h"

#include <algorithm>
#include <atomic>
#include <latch>
#include <thread>

static constexpr size_t kStartCoeff = 1000;

static double RunBenchmark(MetricsCollector *collector,
                           const std::vector<uint64_t> &values, int num_threads,
                           int seconds) {
  std::latch start_latch(1);
  std::atomic<bool> stop_flag{false};
  std::vector<uint64_t> ops(num_threads, 0);
  std::vector<std::thread> threads;

  for (int k = 0; k < num_threads; ++k) {
    threads.emplace_back([&, k]() {
      uint64_t local_count = 0;
      size_t i = (k * kStartCoeff) % values.size();

      start_latch.wait();

      while (!stop_flag.load()) {
        collector->Record(values[i]);
        local_count++;

        i++;
        if (i == values.size()) {
          i = 0;
        }
      }

      ops[k] = local_count;
    });
  }

  auto t0 = std::chrono::steady_clock::now();
  start_latch.count_down();

  std::this_thread::sleep_for(std::chrono::seconds(seconds));
  stop_flag.store(true);

  for (int k = 0; k < num_threads; ++k) {
    threads[k].join();
  }

  auto t1 = std::chrono::steady_clock::now();

  uint64_t total_ops = 0;
  for (int k = 0; k < num_threads; ++k) {
    total_ops += ops[k];
  }

  double duration_sec = std::chrono::duration<double>(t1 - t0).count();

  return total_ops / duration_sec;
}

double MeasurePoint(MetricsCollector *collector,
                    const std::vector<uint64_t> &values, int num_threads) {
  RunBenchmark(collector, values, num_threads, kWarmupSeconds);

  std::vector<double> results;
  for (int i = 0; i < kMeasures; ++i) {
    results.push_back(
        RunBenchmark(collector, values, num_threads, kMeasureSeconds));
  }

  // std::cout << "Count: " << collector->GetSnapshot().count << "\n";

  std::sort(results.begin(), results.end());
  return results[results.size() / 2] * kMeasureCoeff;
}
