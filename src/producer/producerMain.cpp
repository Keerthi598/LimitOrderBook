//
// Created by zekro on 3/12/2026.
//

#include <cmath>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "SharedMemory.hpp"
#include "ipc_protocol.hpp"

const uint32_t PRICE_SCALE = 1000;

void updateData(
    SharedLOB* sharedLOB,
    uint32_t orderId,
    uint32_t qty,
    uint32_t price,
    Side side)
{
    // Wait if Buffer is full
    while (sharedLOB->head.load(std::memory_order_relaxed) -
        sharedLOB->tail.load(std::memory_order_acquire) >= BUFFER_SIZE)
    {
        std::this_thread::yield();
    }

    uint64_t currentHead = sharedLOB->head.load(std::memory_order_relaxed);
    Order& slot = sharedLOB->orders[currentHead % BUFFER_SIZE];

    // Write data to the shared slot
    slot.orderId_ = orderId;
    slot.price_ = price;
    slot.side_ = side;
    slot.quantity_ = qty;

    // Commit teh write
    sharedLOB->head.store(currentHead + 1, std::memory_order_release);
}



int main()
{
    try
    {
        // Initialize the Windows shared segment
        SharedMemory<SharedLOB> shm("Local\\LOB_Bus", true);

        const std::string filePath = DATA_PATH "/IVE_tickbidask.csv";
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            std::cerr << "[ERROR] Could not open data file at: " << filePath << std::endl;
            return 1;
        }

        // --- PRE-LOADING PHASE ---
        // We load data into RAM first to avoid Disk I/O bottlenecks during the benchmark
        // This test is to benchmark the matching speed, at least for now
        std::string line;
        char date[16], time[16];
        double rawPrice, bid, ask;
        int quantity;
        double lastPrice = 0.0;
        uint32_t loadedCount = 0;
        std::vector<Order> preLoadedOrders;
        // I know the file has roughly 12 million entries
        preLoadedOrders.reserve(13'000'000);

        std::cout << "[STATUS] Pre-loading dataset into RAM..." << std::endl;

        while (std::getline(file, line))
        {
            // Parse CSV: Date, Time, Price, Bid, Ask, Quantity
            if (sscanf(line.c_str(), "%[^,],%[^,],%lf,%lf,%lf,%d", date, time, &rawPrice, &bid, &ask, &quantity) == 6)
            {
                auto fixedPrice = static_cast<uint32_t>(std::round(rawPrice * PRICE_SCALE));
                Side side = (rawPrice >= ask) ? Side::BUY : (rawPrice <= bid) ? Side::SELL : (rawPrice >= lastPrice ? Side::BUY : Side::SELL);

                preLoadedOrders.push_back(Order{loadedCount, static_cast<uint32_t>(quantity), fixedPrice, side});
            }

            ++loadedCount;
        }

        std::cout << "[STATUS] Shared Memory Initialized. Ready for Matching." << std::endl;

        // --- BENCHMARK EXECUTION ---
        constexpr uint32_t targetBenchMarkCount = 150'000'000;
        uint32_t processedSoFar = 0;
        auto* sharedLOB = shm.get();

        while (processedSoFar < targetBenchMarkCount)
        {
            // Recycles the dataset (Modulo wrap-around) to simulate high-volume traffic
            for (auto &preLoadedOrder : preLoadedOrders)
            {
                preLoadedOrder.orderId_ = processedSoFar;
                updateData(sharedLOB, preLoadedOrder.orderId_, preLoadedOrder.quantity_, preLoadedOrder.price_, preLoadedOrder.side_);
                ++processedSoFar;
            }
        }

        // Signal completion to the Consumer process
        sharedLOB->producerDone.store(true, std::memory_order_release);
        std::cout << "[STATUS] Producer Run completed. ~150M orders pushed." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error with Producer: " << e.what() << std::endl;
    }

    return 0;
}
