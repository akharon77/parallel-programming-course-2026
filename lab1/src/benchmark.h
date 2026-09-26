#pragma once

#include <vector>

#include "metrics_collector.h"

static constexpr int kWarmupSeconds = 5;
static constexpr int kMeasureSeconds = 5;
static constexpr size_t kMeasures = 5;

double MeasurePoint(MetricsCollector *collector,
                    const std::vector<uint64_t> &values, int num_threads);
