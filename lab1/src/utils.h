#pragma once

#include "metrics_collector.h"

#include <array>
#include <cstdint>

uint64_t ComputePercentile(uint64_t count,
                           const std::array<uint64_t, kNumBuckets> &buckets,
                           double percentile);
