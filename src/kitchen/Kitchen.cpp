#include <iostream>
#include <thread>
#include <chrono>
#include <poll.h>
#include "Kitchen.hpp"
#include "core/Pizza.hpp"

void plazza::kitchen::Kitchen::processEntryPoint()
{
    std::cout << "[K" << this->_id << "] Process entry" << std::endl;

    // Get the read end of the pizza order pipe
    int pizza_fd = pipe->shellToKitchen[0];

    while (true) {
        // Poll for up to 5 seconds for a new pizza order
        struct pollfd pfd;
        pfd.fd = pizza_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        int ret = poll(&pfd, 1, 5000); // 5000ms timeout

        if (ret == 0) {
            // Timeout: no pizza order received after 5 seconds
            std::cout << "[K" << this->_id << "] No pizza received for 5 seconds, shutting down." << std::endl;
            break;
        }
        if (ret < 0) {
            std::cerr << "[K" << this->_id << "] Poll error, shutting down." << std::endl;
            break;
        }
        if (pfd.revents & POLLIN) {
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
    }
    std::cout << "[K" << this->_id << "] Process returning" << std::endl;
}
