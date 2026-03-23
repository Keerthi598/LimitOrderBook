//
// Created by zekro on 3/12/2026.
//

#include "matchingEngine.h"


/**
 * Core Matching Logic
 * Every order is processed through this
 * @param orderId
 * @param quantity
 * @param price
 * @param side
 */
void matchingEngine::processOrder(uint32_t orderId, uint32_t quantity, uint32_t price, Side side)
{
    ++_totalOrders;

    // Side 0 = BID (Buy), Side 1 = ASK (Sell)
    while (quantity > 0)
    {
        // 1. Check for a match
        // If BID, find the best lowest ASK
        // If ASK, find the highest BID
        bool canMatch = (side == Side::BUY)
            ? (!_asks.empty() && _asks.begin()->first <= price)
            : (!_bids.empty() && _bids.begin()->first >= price);

        if (!canMatch)
        {
            // No immediate matches available, so add the order to the book
            addOrderToBook(orderId, quantity, price, side);
            return;
        }

        // 2. Execute the match
        // quantity = matchOrder(orderId, quantity, price, orderType);
        if (side == Side::BUY)
            quantity = matchOrder(_asks, quantity);
        else
            quantity = matchOrder(_bids, quantity);
    }
}


/**
 * Add an order to the book
 * @param orderId
 * @param quantity
 * @param price
 * @param side
 */
void matchingEngine::addOrderToBook(const uint32_t orderId, const uint32_t quantity, const uint32_t price, const Side side)
{
    // Assign a new slot in the OrderPool
    const uint32_t poolIndex = _pool->assignOrder(orderId, quantity, price, side);

    if (side == Side::BUY) // BID
        _bids[price].addOrder(poolIndex);
    else // ASK
        _asks[price].addOrder(poolIndex);
}



