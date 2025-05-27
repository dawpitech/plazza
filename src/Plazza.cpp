/*
** EPITECH PROJECT, 2025
** plazza
** File description:
** Plazza.cpp
*/

#include <csignal>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "Plazza.hpp"
#include "core/Pizza.hpp"
#include "kitchen/Kitchen.hpp"
#include "utils/StringUtils.hpp"

int plazza::Plazza::parseLaunchArgs(const int argc, const char** argv)
{
    if (argc != 4)
        return -1;

    const std::string cookTime = argv[1];
    const std::string cookNumber = argv[2];
    const std::string refillTime = argv[3];

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
    while (this->running)
        this->runCLI();
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
            core::Pizza pizza = {
                .type = core::Pizza::getType(pizzaType),
                .size = core::Pizza::getSize(pizzaSize),
            };

            std::cout << pizza << std::endl;
            std::cout << "Wanting " << quantity << " of them" << std::endl;

            this
        } catch (std::out_of_range& oor) {
            std::cout << oor.what() << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
    }
    this->addNewKitchen();
    return ORDER_COMMAND_STATE::VALID_ORDER;
}

void plazza::Plazza::updateKitchenList()
{
    for (auto it = _kitchens.begin(); it != _kitchens.end(); ) {
        int status;
        if (waitpid(*it, &status, WNOHANG) == 0)
            ++it;
        else
            it = _kitchens.erase(it);
    }
}

void handleSigChild(int signum)
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        std::cout << "[INFO] Subprocess with pid " << pid << " has died." << std::endl;
}

void plazza::Plazza::initChildWatcher()
{
    struct sigaction sa{};
    sa.sa_handler = handleSigChild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);
}

void plazza::Plazza::addNewKitchen()
{
    this->updateKitchenList();
    const pid_t pid = fork();
    const auto kitchenID = this->kitchenID++;

    if (pid == -1) {
        throw std::exception();
    } else if (pid == 0) {
        kitchen::Kitchen kitchen(kitchenID);
        kitchen.processEntryPoint();
        exit(0);
    } else {
        const KitchenData data = {
            .kitchenPID = pid,
            .kitchenSocket = IpcUnixSocket("/tmp/plazza" + kitchenID, IpcUnixSocket::Mode::Server),
        };
        this->_kitchens.push_back(data);
        std::cout << "New process launched with pid: " << pid << std::endl;
    }
}

void plazza::Plazza::runStatus(const Plazza& plazza)
{
    this->updateKitchenList();
    for (const auto [kitchenPID, kitchenSocket] : plazza._kitchens) {
        std::cout << "Kitchen with pid " << kitchenPID << " is alive." << std::endl;
    }
}

void plazza::Plazza::runCLI()
{
    std::cout << "$ > ";

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
