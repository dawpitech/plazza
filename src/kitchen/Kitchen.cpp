/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Kitchen.cpp
*/

#include <iostream>
#include <thread>
#include <chrono>
#include "Kitchen.hpp"
#include "core/Pizza.hpp"

void plazza::kitchen::Kitchen::processEntryPoint()
{
    std::cout << "[K" << this->_id << "] Process entry" << std::endl;

    while (true) {
        try {
            core::Pizza pizza = pipe->receivePizza();
            std::cout << "[K" << this->_id << "] Received pizza: " << pizza << std::endl;

            // Simulate cooking
            std::this_thread::sleep_for(std::chrono::seconds(2));
            pipe->sendCookedPizza(pizza);
            std::cout << "[K" << this->_id << "] Pizza cooked and sent back!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[K" << _id << "] Error: " << e.what() << std::endl;
            break;
        }
    }
    std::cout << "[K" << this->_id << "] Process returning" << std::endl;
}
