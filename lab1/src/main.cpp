#include "benchmark.h"
#include "buffered_collector.h"
#include "generator.h"
#include "global_lock_collector.h"
#include "inconsistency_stress_test.h"
#include "single_thread_collector.h"
#include "striped_lock_collector.h"
#include "thread_local_collector.h"
#include "zero_record_collector.h"

#include <iostream>

static constexpr size_t kNumValues = 2 << 20;
static constexpr uint64_t kMaxZipfValue = 1024;
static constexpr size_t kBaselineNumThreads = 1;

static constexpr size_t kNumThreadsArr[] = {1, 2, 4, 8, 14};

void Baseline(const std::vector<uint64_t> &values) {
  SingleThreadCollector single_thread_collector;
  auto res =
      MeasurePoint(&single_thread_collector, values, kBaselineNumThreads);
  std::cout << "Baseline: " << res << std::endl;
}

template <typename T>
void TestForEachThreadNum(const std::vector<uint64_t> &values) {
  for (size_t num_threads : kNumThreadsArr) {
    T collector;
    auto res = MeasurePoint(&collector, values, num_threads);
    std::cout << "  T = " << num_threads << ": " << res << std::endl;
  }
}

#define STRESS_YES true
#define STRESS_NO false

#ifndef ITER
#define ITER 0
#endif

#define BENCH_FOR_EACH(Class, Stress)                                          \
  do {                                                                         \
    Class collector;                                                           \
    std::cout << #Class ": " << std::endl;                                     \
    if (Stress) {                                                              \
      RunInconsistencyStressTest(&collector, values);                          \
    }                                                                          \
    TestForEachThreadNum<Class>(values);                                       \
  } while (0)

int main() {
  auto values = GenerateZipfValues(kMaxZipfValue, kNumValues);

#if ITER == 0
  Baseline(values);
#elif ITER == 1
  BENCH_FOR_EACH(GlobalLockCollector, STRESS_NO);
  BENCH_FOR_EACH(ZeroRecordLockCollector, STRESS_NO);
#elif ITER == 2
  BENCH_FOR_EACH(StripedLockCollector, STRESS_YES);
#elif ITER == 3
  BENCH_FOR_EACH(ThreadLocalCollector, STRESS_YES);
#elif ITER == 4
  BENCH_FOR_EACH(BufferedCollector, STRESS_YES);
#endif

  return 0;
}
