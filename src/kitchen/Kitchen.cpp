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

void plazza::kitchen::Kitchen::processEntryPoint()
{
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[K" << this->_id << "] Kitchen process entry" << std::endl;

    pollfd pfd = {
        .fd = pipe->shellToKitchen[0],
        .events = POLLIN,
        .revents = 0,
    };

    while (true) {
        pfd.revents = 0;
        const int pollRet = poll(&pfd, 1, 5000);

        if (pollRet == 0) {
            if (debug::DEBUG_MODE)
                std::cout << debug::getTS() << "[K" << this->_id << "] No pizza received for 5 seconds, shutting down." << std::endl;
            break;
        }
        if (pollRet < 0) {
            std::cerr << debug::getTS() << "[K" << this->_id << "] Poll error, shutting down." << std::endl;
            break;
        }
        try {
            core::Pizza pizza = pipe->receivePizza();
            if (debug::DEBUG_MODE)
                std::cout << debug::getTS() << "[K" << this->_id << "] Received pizza: " << pizza << std::endl;

            std::this_thread::sleep_for(pizza.getCookingTime() * this->_cookingTimeFactor);

            pipe->sendCookedPizza(pizza);
                if (debug::DEBUG_MODE)
                    std::cout << debug::getTS() << "[K" << this->_id << "] Pizza cooked and sent back!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[K" << _id << "] Error: " << e.what() << std::endl;
            break;
        }
    }
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[K" << this->_id << "] Kitchen process exiting" << std::endl;
}
