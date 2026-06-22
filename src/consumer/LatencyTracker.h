//
// Created by zekro on 6/21/2026.
//

#ifndef HIGHFREQUENCY_LATENCYTRACKER_H
#define HIGHFREQUENCY_LATENCYTRACKER_H
#include <vector>
#include <windows.h>

/**
 * Class to calculate and print data metrics
 */
class LatencyTracker
{
private:
    std::vector<double> _cycles;
    size_t _currSample{0};

    LARGE_INTEGER _start;
    LARGE_INTEGER _stop;
    LARGE_INTEGER _frequency;

public:
    LatencyTracker(size_t maxSize);

    /**
     * Start the clock
     */
    inline void startClock() {QueryPerformanceCounter(&_start);}

    /**
     * End the clock and record the time taken since the previous start
     */
    inline void stopClockAndRecord()
    {
        QueryPerformanceCounter(&_stop);
        double elapsed_ns = static_cast<double>(_stop.QuadPart - _start.QuadPart) * 1'000'000'000.0 / _frequency.QuadPart;
        _cycles[_currSample] = elapsed_ns;
        ++_currSample;
    }

    void printLatencyReport();
};


#endif //HIGHFREQUENCY_LATENCYTRACKER_H