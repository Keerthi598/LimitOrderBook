//
// Created by zekro on 6/21/2026.
//

#include "LatencyTracker.h"
#include <cmath>
#include <algorithm>
#include <iostream>

/**
 * Constructor
 */
LatencyTracker::LatencyTracker(size_t maxSize)
{
    _cycles.resize(maxSize, 0.0);
    QueryPerformanceFrequency(&_frequency);
}

/**
 * Print the latency metrics
 */
void LatencyTracker::printLatencyReport()
{
    std::sort(_cycles.begin(), _cycles.begin() + _currSample);
    double min_lat = _cycles[0];
    double max_lat = _cycles[_currSample - 1];

    double mean_lat = 0.0;
    for (size_t i = 0; i < _cycles.size(); ++i)
        mean_lat += _cycles[i];
    mean_lat /= _currSample;

    auto get_percentile = [&](double percentile) -> double
    {
        size_t idx = static_cast<size_t>(std::ceil((percentile / 100.0) * _currSample)) - 1;
        idx = std::min(idx, _currSample - 1);
        return _cycles[idx];
    };

    std::cout << "\n==========================================\n";
    std::cout << "  PERFORMANCE METRICS\n";
    std::cout << "==========================================\n";
    std::cout << "Min Latency    : " << min_lat << " ns\n";
    std::cout << "Average (Avg)  : " << mean_lat << " ns\n";
    // Median metrics removed due to the loss of precision below 100ns showing this as 0ns
    // Due to hardware limitations on windows
    // std::cout << "P50 (Median)   : " << get_percentile(50.0) << " ns\n";
    std::cout << "P90            : " << get_percentile(90.0) << " ns\n";
    std::cout << "P99            : " << get_percentile(99.0) << " ns\n";
    std::cout << "P99.9          : " << get_percentile(99.9) << " ns\n";
    std::cout << "P99.99         : " << get_percentile(99.99) << " ns\n";
    std::cout << "Max Latency    : " << max_lat << " ns\n";
    std::cout << "==========================================\n";
}