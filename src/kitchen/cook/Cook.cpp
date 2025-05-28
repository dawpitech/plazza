/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Cook.cpp
*/

#include "Cook.hpp"

plazza::kitchen::Cook::Cook(std::unordered_map<core::Ingredients, unsigned>& ingredientStock,
    std::mutex& ingredientMtx,
    utils::ThreadSafeQueue<core::Pizza>& waitingOrders,
    utils::ThreadSafeQueue<core::Pizza>& finishedOrders)
    : _thread(nullptr)
    , _ingredientStock(ingredientStock)
    , _ingredientMutex(ingredientMtx)
    , _waitingOrders(waitingOrders)
    , _finishedOrders(finishedOrders)
{
}

void plazza::kitchen::Cook::letHimCook()
{
    this->_thread = std::make_unique<std::thread>(threadEntryPoint, std::ref(*this));
}

[[noreturn]] void plazza::kitchen::Cook::threadEntryPoint(Cook& self)
{
    //std::cout << "[ C ] " << "Cook thread entrypoint" << std::endl;
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
        //std::cout << "Thread cooking: " << pizza << std::endl;
        std::this_thread::sleep_for(pizza.getCookingTime());
        self._finishedOrders.push(pizza);
    }
}
