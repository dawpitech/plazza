/*
** EPITECH PROJECT, 2025
** Kitchen
** File description:
** Kitchen.cpp
*/

#include <iomanip>
#include <iostream>
#include <poll.h>
#include <thread>

#include "Kitchen.hpp"
#include "core/Pizza.hpp"
#include "utils/Debug.hpp"

plazza::kitchen::Kitchen::Kitchen(const int id, std::unique_ptr<ipc::TuyauAPizza> p,
    const float cookingTimeFactor,
    const unsigned int numberOfCooks,
    const unsigned int refillDelay)
    : _id(id)
    , pipe(std::move(p))
    , _cookingTimeFactor(cookingTimeFactor)
    , _numberOfCooks(numberOfCooks)
    ,  _refillDelay(refillDelay)
{
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Dough, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Tomato, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Gruyere, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Ham, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Mushrooms, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Steak, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::Eggplant, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::GoatCheese, 5));
    this->_ingredientStock.insert(std::make_pair(core::Ingredients::ChiefLove, 5));
    for (unsigned int i = 0; i < this->_numberOfCooks; i++)
        this->_cooks.emplace_back(std::make_unique<Cook>(
            this->_ingredientStock,
            this->_ingredientMutex,
            this->_waitingOrders,
            this->_finishedOrders));
}

void plazza::kitchen::Kitchen::processEntryPoint()
{
    pollfd pfd = {
        .fd = pipe->shellToKitchen[0],
        .events = POLLIN,
        .revents = 0,
    };
    auto lastOrderInTime = std::chrono::system_clock::now();


    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[K" << this->_id << "] Kitchen process entry" << std::endl;

    for (const auto& cook : this->_cooks)
        cook->letHimCook();

    while (true) {
        pfd.revents = 0;
        const int pollRet = poll(&pfd, 1, 0);

        const auto now = std::chrono::system_clock::now();
        if (pollRet < 0) {
            std::cerr << debug::getTS() << "[K" << this->_id << "] Poll error, shutting down." << std::endl;
            break;
        }
        if (pollRet == 0)
        {
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastOrderInTime).count() > TIMEOUT) {
                if (debug::DEBUG_MODE)
                    std::cout << debug::getTS() << "[K" << this->_id << "] No pizza received for " << TIMEOUT << " seconds, shutting down." << std::endl;
                break;
            }
            while (!this->_finishedOrders.empty()) {
                this->pipe->sendCookedPizza(this->_finishedOrders.get());
                if (debug::DEBUG_MODE)
                    std::cout << debug::getTS() << "[K" << this->_id << "] Pizza cooked and sent back!" << std::endl;
            }
            continue;
        }

        lastOrderInTime = std::chrono::system_clock::now();
        try {
            core::Pizza pizza = pipe->receivePizza();
            if (debug::DEBUG_MODE)
                std::cout << debug::getTS() << "[K" << this->_id << "] Received pizza: " << pizza << std::endl;
            this->_waitingOrders.push(pizza);
        } catch (const std::exception& e) {
            std::cerr << "[K" << _id << "] Error: " << e.what() << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[K" << this->_id << "] Kitchen process exiting" << std::endl;
}
