//
// Created by zekro on 3/14/2026.
//

#include "OrderPool.h"

#include <stdexcept>

/**
 * Constructor
 * @param maxSize
 */
OrderPool::OrderPool(size_t maxSize)
{
    _orders.resize(maxSize);
    _availableIndices.reserve(maxSize);

    for (int i{0}; i < maxSize; i++)
        _availableIndices.push_back(i);
}

/**
 * Allocate an index and return it
 * @return Index
 */
uint32_t OrderPool::getAvailableIndex()
{
    if (_availableIndices.empty())
        throw std::runtime_error("Pool Exhausted");

    const uint32_t freeIndex = _availableIndices.back();
    _availableIndices.pop_back();

    return freeIndex;
}

/**
 * Free up a used order index
 * @param index
 */
void OrderPool::freeIndex(const uint32_t& index)
{
    _availableIndices.push_back(index);
}

/**
 * Add a new order to the pool and return its index
 * @param orderId
 * @param quantity
 * @param price
 * @param side
 * @return Index
 */
uint32_t OrderPool::assignOrder(const uint32_t& orderId, const uint32_t& quantity, const uint32_t& price, const Side& side)
{
    if (_availableIndices.empty())
        throw std::runtime_error("Pool Exhausted");

    const uint32_t freeIndex = _availableIndices.back();
    _availableIndices.pop_back();

    _orders[freeIndex].quantity_ = quantity;
    _orders[freeIndex].price_ = price;
    _orders[freeIndex].orderId_ = orderId;
    _orders[freeIndex].side_ = side;

    return freeIndex;
}