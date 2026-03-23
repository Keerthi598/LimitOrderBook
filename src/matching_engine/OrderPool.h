//
// Created by zekro on 3/14/2026.
//

#ifndef HIGHFREQUENCY_ORDERPOOL_H
#define HIGHFREQUENCY_ORDERPOOL_H

#include <stack>
#include <vector>
#include "../../include/ipc_protocol.hpp"

/**
 * A pool of pre-allocated empty orders
 */
class OrderPool
{
private:
    std::vector<Order> _orders; // Internal order storage vector
    std::vector<uint32_t> _availableIndices; // Free order indices

public:
    OrderPool(size_t maxSize);
    OrderPool() = delete;

    uint32_t getAvailableIndex();

    void freeIndex(const uint32_t& index);

    Order* getOrder(const uint32_t& index) {return &_orders[index];}

    uint32_t assignOrder(const uint32_t& orderId, const uint32_t& quantity, const uint32_t& price, const Side& side);
};



#endif //HIGHFREQUENCY_ORDERPOOL_H
