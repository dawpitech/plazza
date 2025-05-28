/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Cook.cpp
*/

#include <iostream>

#include "Cook.hpp"

#include "utils/Debug.hpp"

plazza::kitchen::Cook::Cook(std::unordered_map<core::Ingredients, unsigned>& ingredientStock,
                            std::mutex& ingredientMtx,
                            utils::ThreadSafeQueue<core::Pizza>& waitingOrders,
                            utils::ThreadSafeQueue<core::Pizza>& finishedOrders,
                            const float cookingTimeFactor)
    : _thread(nullptr)
    , _ingredientStock(ingredientStock)
    , _ingredientMutex(ingredientMtx)
    , _waitingOrders(waitingOrders)
    , _finishedOrders(finishedOrders)
    , _cookingTimeFactor(cookingTimeFactor)
{
}

void plazza::kitchen::Cook::letHimCook()
{
    this->_thread = std::make_unique<std::thread>(threadEntryPoint, std::ref(*this));
}

[[noreturn]] void plazza::kitchen::Cook::threadEntryPoint(Cook& self)
{
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[C?] Cook thread launched" << std::endl;
    while (true)
    {
        if (self._waitingOrders.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        const auto maybePizza = self._waitingOrders.maybeGet();
        if (!maybePizza.has_value())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        core::Pizza pizza = *maybePizza;
        if (debug::DEBUG_MODE)
            std::cout << debug::getTS() << "[C?] Cooking pizza: " << pizza << std::endl;
        std::this_thread::sleep_for(pizza.getCookingTime() * self._cookingTimeFactor);
        if (debug::DEBUG_MODE)
            std::cout << debug::getTS() << "[C?] Done cooking pizza" << std::endl;
        self._finishedOrders.push(pizza);
    }
}
