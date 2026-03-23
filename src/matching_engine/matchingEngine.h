//
// Created by zekro on 3/12/2026.
//

#ifndef HIGHFREQUENCY_MATCHINGENGINE_H
#define HIGHFREQUENCY_MATCHINGENGINE_H
#include <cstdint>
#include <functional>
#include <list>
#include <map>

#include "ipc_protocol.hpp"
#include "OrderPool.h"


/**
 * The matching engine
 */
class matchingEngine
{
private:
    /**
    * The orders at a specific price.
    * Prioritizes older orders
    */
    class PriceLevel {
    private:
        std::vector<uint32_t> _orders;
        size_t _head = 0;
        size_t _tail = 0;

    public:
        PriceLevel() {
            _orders.reserve(64); // Pre-allocate small initial depth
        }

        bool empty() const { return _head == _tail; }

        uint32_t front() const { return _orders[_head]; }

        void addOrder(uint32_t orderIndex) {
            // Push_back cos pricelevels can be frequently created and erased
            _orders.push_back(orderIndex);
            _tail++;
        }

        void deleteOrder() {
            if (!empty()) _head++;
        }
    };

    std::map<uint32_t, PriceLevel, std::greater<>> _bids;
    std::map<uint32_t, PriceLevel, std::less<>> _asks;

    OrderPool* _pool;

    uint64_t _totalVolume{0};
    uint64_t _matchCount{0};
    uint64_t _totalOrders{0};

    /**
      * Match a given a order.
      * Called only when a match is found
      * @tparam T The correct book to match from
      * @param targetBook
      * @param quantity
      * @return quantity- 0 if fully matched, else remaining qty
      */
    template <typename T>
    uint32_t matchOrder(T& targetBook, uint32_t quantity)
    {
        auto bestLevelEntry = targetBook.begin();
        uint32_t matchIndex = bestLevelEntry->second.front();
        auto *matchedOrder = _pool->getOrder(matchIndex);
        ++_matchCount;

        if (matchedOrder->quantity_ <= quantity)
        {
            // The matched order is fully completed
            quantity -= matchedOrder->quantity_;
            bestLevelEntry->second.deleteOrder();
            _totalVolume += matchedOrder->quantity_;

            if (bestLevelEntry->second.empty())
            {
                // Remove the price level if no other orders at this price
                targetBook.erase(bestLevelEntry);
            }
            _pool->freeIndex(matchIndex);
        }
        else
        {
            matchedOrder->quantity_ -= quantity;
            _totalVolume += quantity;
            quantity = 0;
        }

        return quantity;
    }

    void addOrderToBook(
    uint32_t orderId,
    uint32_t quantity,
    uint32_t price,
    Side side);

public:
    matchingEngine(OrderPool* pool) {_pool = pool;}
    matchingEngine() = delete;

    uint64_t getTotalVolume() const { return _totalVolume; }
    uint64_t getMatchCount() const { return _matchCount; }
    uint64_t getTotalOrders() const { return _totalOrders; }

    void processOrder(
    uint32_t orderId,
    uint32_t quantity,
    uint32_t price,
    Side side);
};

#endif //HIGHFREQUENCY_MATCHINGENGINE_H