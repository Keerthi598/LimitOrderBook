//
// Created by zekro on 3/12/2026.
//

#ifndef HIGHFREQUENCY_IPC_PROTOCOL_H
#define HIGHFREQUENCY_IPC_PROTOCOL_H

#include <atomic>

enum struct Side : uint8_t
{
    BUY = 0,
    SELL = 1
};

struct alignas(16) Order
{
    uint32_t orderId_{0};
    uint32_t quantity_{0};
    uint32_t price_{0};

    // 0-Bid, 1-Ask
    Side side_ {Side::SELL};
    uint8_t padding_[3];
};

constexpr size_t BUFFER_SIZE = 4096;

struct SharedLOB
{
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
    alignas(64) std::atomic<bool> producerDone{false};

    Order orders[BUFFER_SIZE];
};

#endif //HIGHFREQUENCY_IPC_PROTOCOL_H