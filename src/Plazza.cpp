/*
** EPITECH PROJECT, 2025
** plazza
** File description:
** Plazza.cpp
*/

#include <csignal>
#include <iostream>
#include <map>
#include <poll.h>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "Plazza.hpp"
#include "core/Pizza.hpp"
#include "IPC/TuyauAPizza.hpp"
#include "kitchen/Kitchen.hpp"
#include "utils/Debug.hpp"
#include "utils/StringUtils.hpp"

int plazza::Plazza::parseLaunchArgs(const int argc, const char** argv)
{
    if (argc != 4 && argc != 5)
        return -1;

    const std::string cookTime = argv[1];
    const std::string cookNumber = argv[2];
    const std::string refillTime = argv[3];
    if (argc == 5) {
        const std::string debugModeState = argv[4];
        debug::DEBUG_MODE = debugModeState == "-d";
    }

    try {
        this->cookingTimeFactor = std::stof(cookTime);
        this->cooksPerKitchen = std::stoul(cookNumber);
        this->refillDelay = std::stoul(refillTime);
    } catch (std::invalid_argument&) {
        return -1;
    } catch (std::out_of_range&) {
        return -1;
    }

    if (this->cooksPerKitchen > 32 || this->cooksPerKitchen == 0)
        return -1;
    if (this->cookingTimeFactor <= 0)
        return -1;
    return 0;
}

void plazza::Plazza::run()
{
    initChildWatcher();
    this->_reception = std::make_unique<Reception>
        (this->cookingTimeFactor, this->cooksPerKitchen, this->refillDelay);
    if (!debug::DEBUG_MODE)
        std::cout << "$ > " << std::flush;
    while (this->running) {
        std::vector<pollfd> pollfds;
        pollfd stdin_pfd = {STDIN_FILENO, POLLIN, 0};
        pollfds.push_back(stdin_pfd);

        for (const auto& kitchenFD : this->_reception->getKitchenPipeFD()) {
            pollfd pfd = {
                .fd = kitchenFD,
                .events = POLLIN,
                .revents = 0,
            };
            pollfds.push_back(pfd);
        }

        if (poll(pollfds.data(), pollfds.size(), -1) > 0) {
            for (size_t i = 0; i < pollfds.size(); ++i)
            {
                if (!(pollfds[i].revents & POLLIN))
                    continue;
                if (i == 0) {
                    this->runCLI();
                    if (!debug::DEBUG_MODE)
                        std::cout << "$ > " << std::flush;
                    break;
                }
                this->_reception->checkForCookedPizzas(pollfds[i].fd);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void plazza::Plazza::runQuit(Plazza& plazza)
{
    this->running = false;
}

plazza::Plazza::ORDER_COMMAND_STATE plazza::Plazza::parsePizzaOrder(Plazza& plazza, const std::string& rawOrder)
{
    const std::vector<std::string> orders = utils::StringUtils::split(rawOrder, ';');
    for (const auto& order : orders) {
        const auto params = utils::StringUtils::split(order, ' ');
        if (params.size() != 3)
            return ORDER_COMMAND_STATE::NOT_AN_ORDER;

        const std::string& pizzaType = utils::StringUtils::applyCamelCase(params.at(0));
        const std::string& pizzaSize = params.at(1);
        const std::string& pizzaQuantity = params.at(2);

        if (!std::regex_match(pizzaType, std::regex("^[a-zA-Z]+$"))) {
            std::cout << "Given pizza name format is invalid." << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
        if (!std::regex_match(pizzaSize, std::regex("^(S|M|L|XL|XXL)$"))) {
            std::cout << "Given pizza size is invalid." << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
        if (!std::regex_match(pizzaQuantity, std::regex("^x[1-9][0-9]*$"))) {
            std::cout << "Given pizza quantity is invalid." << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
        const auto quantity = std::stoi(&pizzaQuantity.at(1));
        if (quantity > 32) {
            std::cout << "Max order size is 32 per pizza type." << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
        try {
            const core::Pizza pizza = {
                .type = core::Pizza::getType(pizzaType),
                .size = core::Pizza::getSize(pizzaSize),
            };

            this->_reception->updateKitchenList();
            for (int i = 0; i < quantity; i++)
                this->_reception->orderPizza(pizza);
        } catch (std::out_of_range& oor) {
            std::cout << oor.what() << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
    }
    return ORDER_COMMAND_STATE::VALID_ORDER;
}

void handleSigChild([[maybe_unused]] int signum)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void plazza::Plazza::initChildWatcher()
{
    struct sigaction sa{};
    sa.sa_handler = handleSigChild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);
}

void plazza::Plazza::runStatus(const Plazza& plazza) const
{
    this->_reception->printKitchenStatus();
}

void plazza::Plazza::runCLI()
{
    if (std::string input; !std::getline(std::cin, input)) {
        this->running = false;
    } else {
        if (input.empty()) return;
        try {
            this->STATIC_COMMANDS.at(input)(*this);
        } catch (std::out_of_range&) {
            if (this->parsePizzaOrder(*this, input) == ORDER_COMMAND_STATE::NOT_AN_ORDER)
                std::cout << "Unknown command." << std::endl;
        }
    }
}
