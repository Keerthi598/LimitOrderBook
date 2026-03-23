//
// Created by zekro on 3/12/2026.
//

#include <iomanip>
#include <iostream>
#include <thread>
#include <chrono>
#include <ratio>

#include "matchingEngine.h"
#include "OrderPool.h"
#include "SharedMemory.hpp"

int main()
{
    try
    {
        // 1. Engine Initialization
        // Pre-allocating the OrderPool to avoid dynamic memory allocation
        uint32_t poolSize = BUFFER_SIZE << 16;
        OrderPool pool(poolSize);
        matchingEngine engine(&pool);

        // 2. Shared Memory Attachment
        // "false" cos we are attaching to an existing segment created by the producer
        SharedMemory<SharedLOB> shm("Local\\LOB_Bus", false);
        auto* shmBuffer = shm.get();

        uint32_t consumedIndex = 0;
        uint32_t totalProcessed = 0;

        std::cout << "[STATUS] Consumer attached. Awaiting Producer data..." << std::endl;

        // 3. Benchmarking Timers
        auto startTime = std::chrono::high_resolution_clock::now();
        auto lastReportTime = startTime;
        uint32_t lastReportCount = 0;

        // 4. The Hot Loop
        while (true)
        {
            // Wait if the buffer is empty and producer is still active
            while (shmBuffer->head.load(std::memory_order_acquire) == consumedIndex &&
                shmBuffer->producerDone.load(std::memory_order_acquire) == false)
            {
                std::this_thread::yield();
            }

            uint32_t currentHead = shmBuffer->head.load(std::memory_order_acquire);

            // Exit condition: Break if the producer is done and the buffer is drained
            if (shmBuffer->producerDone.load(std::memory_order_acquire) &&
                consumedIndex >= currentHead)
            {
                break;
            }

            // 5. Process Order
            // Map the global index to the circular buffer index
            Order& slot = shmBuffer->orders[consumedIndex % BUFFER_SIZE];

            // Match the order
            engine.processOrder(slot.orderId_, slot.quantity_, slot.price_, slot.side_);

            consumedIndex++;
            totalProcessed++;

            // 6. Lives metrics every second, but check if time passed only every 10M orders
            if (consumedIndex % 10000000 == 0)
            {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastReportTime).count();

                if (elapsedMs >= 1000)
                {
                    double mps = (consumedIndex - lastReportCount) / (elapsedMs / 1000.0) / 1e6;
                    std::cout << "[LIVE] Processed: " << consumedIndex
                              << " | Current Speed: " << std::fixed << std::setprecision(2) << mps << " M/s\n";

                    lastReportTime = now;
                    lastReportCount = consumedIndex;
                }
            }

            // 7. Tail Update
            shmBuffer->tail.store(consumedIndex, std::memory_order_release);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

        // --- FINAL PERFORMANCE REPORT ---
        double avgLatencyNs = static_cast<double>(totalNs) / totalProcessed;
        double throughputMps = (static_cast<double>(totalProcessed) / totalNs) * 1e9 / 1e6;

        std::cout << "\n==========================================\n";
        std::cout << "  PERFORMANCE METRICS (N = " << totalProcessed << ")\n";
        std::cout << "==========================================\n";
        std::cout << "Total Time  : " << std::fixed << std::setprecision(2) << totalNs / 1e6 << " ms\n";
        std::cout << "Avg Latency : " << std::fixed << std::setprecision(2) << avgLatencyNs << " ns/order\n";
        std::cout << "Throughput  : " << std::fixed << std::setprecision(2) << throughputMps << " million orders/sec\n";
        std::cout << "==========================================\n";
        std::cout << "==========================================" << std::endl;
        std::cout << "           MATCHING ENGINE METRICS   " << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Total Matches Executed    : " << engine.getMatchCount() << std::endl;
        std::cout << "Total Volume Traded       : " << engine.getTotalVolume() << std::endl;
        std::cout << "Total Orders Processed    : " << engine.getTotalOrders() << std::endl;
        std::cout << "==========================================" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
    }

    return 0;
}