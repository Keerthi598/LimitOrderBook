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
#include "LatencyTracker.h"

int main()
{
    try
    {
        // 1. Engine Initialization
        // Pre-allocating the OrderPool to avoid dynamic memory allocation
        uint32_t poolSize = BUFFER_SIZE << 16;
        OrderPool pool(poolSize);
        matchingEngine engine(&pool);
        LatencyTracker tracker{trackerSize};

        // 2. Shared Memory Attachment
        // "false" cos we are attaching to an existing segment created by the producer
        SharedMemory<SharedLOB> shm("Local\\LOB_Bus", false);
        auto* shmBuffer = shm.get();

        uint32_t consumedIndex = 0;
        uint32_t totalProcessed = 0;

        std::cout << "[STATUS] Consumer attached. Awaiting Producer data..." << std::endl;


        // 3. The Hot Loop
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

            // 4. Begin the clock to time this order's processing
            tracker.startClock();

            // 5. Process Order
            // Map the global index to the circular buffer index
            Order& slot = shmBuffer->orders[consumedIndex % BUFFER_SIZE];

            // Match the order
            engine.processOrder(slot.orderId_, slot.quantity_, slot.price_, slot.side_);

            consumedIndex++;
            totalProcessed++;

            // 6. Tail Update
            shmBuffer->tail.store(consumedIndex, std::memory_order_release);

            // 7. Stop this clock and record teh time taken
            tracker.stopClockAndRecord();

            // Status update every 10 million orders
            if (consumedIndex % 10000000 == 0) {
                uint32_t millDone = consumedIndex / 1000000;
                std::cout << "[STATUS] " << millDone << " Million Orders Processed\n";
            }
        }

        // 8. Final metrics report
        tracker.printLatencyReport();
        std::cout << "==========================================\n";
        std::cout << "           MATCHING ENGINE METRICS   \n";
        std::cout << "==========================================\n";
        std::cout << "Total Matches Executed    : " << engine.getMatchCount() << "\n";
        std::cout << "Total Volume Traded       : " << engine.getTotalVolume() << "\n";
        std::cout << "Total Orders Processed    : " << engine.getTotalOrders() << "\n";
        std::cout << "==========================================\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
    }

    return 0;
}