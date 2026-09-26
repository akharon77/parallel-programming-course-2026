#pragma once

#include "metrics_collector.h"

#include <vector>

void RunInconsistencyStressTest(MetricsCollector *collector,
                                const std::vector<uint64_t> &values);
