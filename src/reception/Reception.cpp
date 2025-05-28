/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Reception.cpp
*/

#include <iostream>
#include <sys/wait.h>

#include "Reception.hpp"
#include "kitchen/Kitchen.hpp"
#include "utils/Debug.hpp"

std::vector<int> plazza::Reception::getKitchenPipeFD() const
{
    std::vector<pid_t> list;
    for (const auto& kitchen : this->_kitchens)
        list.push_back(kitchen.pipe->kitchenToShell[0]);
    return list;
}

void plazza::Reception::orderPizza(const core::Pizza pizza)
{
    this->updateKitchenList();
    auto& availableKitchen = this->getNextKitchen();

    availableKitchen.pipe->sendPizza(pizza);
    availableKitchen.pendingPizzas.emplace_back(pizza);
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[R-] Pizza order sent to kitchen " << availableKitchen.kitchenID << std::endl;
}

void plazza::Reception::checkForCookedPizzas(const int kitchenPipeFD)
{
    for (auto& kitchen : this->_kitchens) {
        if (kitchen.pipe->kitchenToShell[0] != kitchenPipeFD)
            continue;
        try {
            const core::Pizza cooked = kitchen.pipe->receiveCookedPizza();
            kitchen.pendingPizzas.erase(kitchen.pendingPizzas.begin());
            if (debug::DEBUG_MODE)
                std::cout << debug::getTS() << "[R-] Pizza order completed, here is your " << cooked << std::endl;
            return;
        } catch ([[maybe_unused]] ipc::TuyauAPizza::KitchenClosedException& kce) {
            this->updateKitchenList();
            break;
        }
    }
    throw std::runtime_error("Given PipeFD isn't valid");
}

void plazza::Reception::printKitchenStatus()
{
    this->updateKitchenList();
    for (const auto& kitchen : this->_kitchens) {
        if (debug::DEBUG_MODE)
            std::cout << debug::getTS() << "[R-] ";
        std::cout << "Kitchen " << kitchen.kitchenID << " is alive";
        if (kitchen.pendingPizzas.empty()) {
            std::cout << " without any order." << std::endl;
            continue;
        }
        std::cout << " with " << kitchen.pendingPizzas.size() << " pizza pending:" << std::endl;
        for (const auto& pizza : kitchen.pendingPizzas) {
            if (debug::DEBUG_MODE)
                std::cout << debug::getTS() << "[-R] ";
            std::cout << "\t - " << pizza << std::endl;
        }
    }
    if (this->_kitchens.empty())
    {
        if (debug::DEBUG_MODE)
            std::cout << debug::getTS() << "[-R] ";
        std::cout << "No kitchen currently open" << std::endl;
    }
}

plazza::Reception::KitchenData& plazza::Reception::getNextKitchen()
{
    for (auto& kitchen : this->_kitchens) {
        if (kitchen.pendingPizzas.size() >= this->SLOTS)
            continue;
        return kitchen;
    }
    this->launchNewKitchen();
    return this->_kitchens.back();
}

void plazza::Reception::updateKitchenList()
{
    for (auto it = _kitchens.begin(); it != _kitchens.end(); ) {
        int status;
        if (waitpid(it->kitchenPID, &status, WNOHANG) == 0)
            ++it;
        else
            it = _kitchens.erase(it);
    }
}

void plazza::Reception::launchNewKitchen()
{
    this->updateKitchenList();
    auto pipe = std::make_unique<ipc::TuyauAPizza>();
    const pid_t pid = fork();
    const auto kitchenID = this->newKitchenID++;

    if (pid == -1)
        throw std::runtime_error("Failed to fork kitchen");

    if (pid == 0) {
        pipe->closeReceptionSidePipes();
        kitchen::Kitchen kitchen(kitchenID, std::move(pipe), this->COOKING_TIME_FACTOR);
        kitchen.processEntryPoint();
        exit(0);
    }

    pipe->closeKitchenSidePipes();
    this->_kitchens.emplace_back(pid, std::move(pipe), kitchenID);
    if (debug::DEBUG_MODE)
        std::cout << debug::getTS() << "[R-] New kitchen launched. PID:" << pid << " ID: " << kitchenID << std::endl;
}

plazza::Reception::KitchenData::KitchenData(const pid_t pid, std::unique_ptr<ipc::TuyauAPizza>&& p, const int id)
    : kitchenID(id), kitchenPID(pid), pipe(std::move(p)) {}

plazza::Reception::KitchenData::KitchenData(KitchenData&& other) noexcept
    : kitchenID(other.kitchenID), kitchenPID(other.kitchenPID), pipe(std::move(other.pipe)), pendingPizzas(std::move(other.pendingPizzas)) {}

plazza::Reception::KitchenData& plazza::Reception::KitchenData::operator=(KitchenData&& other) noexcept {
    if (this != &other) {
        kitchenID = other.kitchenID;
        kitchenPID = other.kitchenPID;
        pipe = std::move(other.pipe);
        pendingPizzas = std::move(other.pendingPizzas);
    }
    return *this;
}
