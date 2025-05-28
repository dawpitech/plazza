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
#include <thread>
#include <map>
#include <cstring>

#include "Plazza.hpp"
#include "core/Pizza.hpp"
#include "kitchen/Kitchen.hpp"
#include "utils/StringUtils.hpp"
#include "IPC/PizzaPipe.hpp"

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

#include <poll.h>

void plazza::Plazza::run()
{
    initChildWatcher();
    std::cout << "$ > " << std::flush;
    while (this->running) {
        // Prepare pollfd array: stdin + all kitchen pipes
        std::vector<struct pollfd> pollfds;

        // STDIN
        struct pollfd stdin_pfd = {STDIN_FILENO, POLLIN, 0};
        pollfds.push_back(stdin_pfd);

        // All kitchen pipes (read end for cooked pizzas)
        for (auto &kitchen : _kitchens) {
            struct pollfd pfd;
            pfd.fd = kitchen.pipe->kitchenToShell[0];
            pfd.events = POLLIN;
            pfd.revents = 0;
            pollfds.push_back(pfd);
        }

        // poll() with timeout (e.g., 500ms for responsiveness)
        int ret = poll(pollfds.data(), pollfds.size(), 500);

        if (ret > 0) {
            // Check all fds
            for (size_t i = 0; i < pollfds.size(); ++i) {
                if (pollfds[i].revents & POLLIN) {
                    if (i == 0) {
                        // STDIN ready: runCLI (single-shot, then break to re-poll)
                        this->runCLI();
                        break;
                    } else {
                        // One of the kitchen pipes is ready
                        auto &kitchen = _kitchens[i - 1];
                        try {
                            core::Pizza cooked = kitchen.pipe->receiveCookedPizza();
                            auto &pending = _pendingPizzasPerKitchen[kitchen.kitchenPID];
                            auto it = std::find_if(pending.begin(), pending.end(), [&](const core::Pizza& p) {
                                return p.type == cooked.type && p.size == cooked.size;
                            });
                            if (it != pending.end())
                                pending.erase(it);
                            std::cout << "[ASYNC] Received cooked pizza from kitchen " << kitchen.kitchenPID << ": " << cooked << std::endl;
                        } catch (const std::exception&) {
                            // Ignore: could be spurious poll
                        }
                    }
                }
            }
        }
        // If ret == 0: timeout, loop again (can add a small sleep if you want)
        // No need for checkForCookedPizzas() anymore, it's inline above
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
            core::Pizza pizza = {
                .type = core::Pizza::getType(pizzaType),
                .size = core::Pizza::getSize(pizzaSize),
            };
            // std::cout << pizza << std::endl;
            // std::cout << "Wanting " << quantity << " of them" << std::endl;

            if (_kitchens.empty())
                this->addNewKitchen();

            for (int i = 0; i < quantity; ++i) {
                try {
                    _kitchens.front().pipe->sendPizza(pizza);
                    _pendingPizzasPerKitchen[_kitchens.front().kitchenPID].push_back(pizza);
                    std::cout << "Sent pizza order to kitchen PID " << _kitchens.front().kitchenPID << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to send pizza to kitchen: " << e.what() << std::endl;
                    return ORDER_COMMAND_STATE::INVALID_ORDER;
                }
            }

            // *** REMOVE BLOCKING RECEIVE LOOP HERE ***
            // cooked pizzas will be handled asynchronously by checkForCookedPizzas()
        } catch (std::out_of_range& oor) {
            std::cout << oor.what() << std::endl;
            return ORDER_COMMAND_STATE::INVALID_ORDER;
        }
    }
    return ORDER_COMMAND_STATE::VALID_ORDER;
}

void plazza::Plazza::updateKitchenList()
{
    for (auto it = _kitchens.begin(); it != _kitchens.end(); ) {
        int status;
        if (waitpid(it->kitchenPID, &status, WNOHANG) == 0)
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
    auto pipe = std::make_unique<plazza::ipc::PizzaPipe>();
    const pid_t pid = fork();
    const auto kitchenID = this->kitchenID++;

    if (pid == -1) {
        throw std::runtime_error("Failed to fork kitchen");
    } else if (pid == 0) {
        // Child: kitchen process
        pipe->closeShellEnds(); // Only use kitchen ends
        kitchen::Kitchen kitchen(kitchenID, std::move(pipe));
        kitchen.processEntryPoint();
        exit(0);
    } else {
        // Parent: shell process
        pipe->closeKitchenEnds(); // Only use shell ends
        _kitchens.emplace_back(pid, std::move(pipe));
        std::cout << "New process launched with pid: " << pid << std::endl;
    }
}

void plazza::Plazza::runStatus(const Plazza& plazza)
{
    this->updateKitchenList();
    for (const auto& kitchen : plazza._kitchens) {
        std::cout << "Kitchen with pid " << kitchen.kitchenPID << " is alive." << std::endl;
        const auto& pending = _pendingPizzasPerKitchen[kitchen.kitchenPID];
        if (pending.empty()) {
            std::cout << "  No pizzas currently cooking." << std::endl;
        } else {
            std::cout << "  Pizzas currently cooking:" << std::endl;
            for (const auto& pizza : pending)
                std::cout << "    - " << pizza << std::endl;
        }
    }
}

void plazza::Plazza::checkForCookedPizzas()
{
    return;
    for (auto& kitchen : _kitchens) {
        while (true) {
            try {
                core::Pizza cooked = kitchen.pipe->receiveCookedPizza();
                auto& pending = _pendingPizzasPerKitchen[kitchen.kitchenPID];
                // Remove the first matching pizza (by type and size)
                auto it = std::find_if(pending.begin(), pending.end(), [&](const core::Pizza& p) {
                    return p.type == cooked.type && p.size == cooked.size;
                });
                if (it != pending.end())
                    pending.erase(it);
                std::cout << "[ASYNC] Received cooked pizza from kitchen " << kitchen.kitchenPID << ": " << cooked << std::endl;
            } catch (const std::exception&) {
                break;
            }
        }
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

// Plazza::KitchenData implementation
plazza::Plazza::KitchenData::KitchenData(pid_t pid, std::unique_ptr<plazza::ipc::PizzaPipe>&& p)
    : kitchenPID(pid), pipe(std::move(p)) {}

plazza::Plazza::KitchenData::KitchenData(KitchenData&& other) noexcept
    : kitchenPID(other.kitchenPID), pipe(std::move(other.pipe)) {}

plazza::Plazza::KitchenData& plazza::Plazza::KitchenData::operator=(KitchenData&& other) noexcept {
    if (this != &other) {
        kitchenPID = other.kitchenPID;
        pipe = std::move(other.pipe);
    }
    return *this;
}
