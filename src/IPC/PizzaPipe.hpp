#pragma once

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "core/Pizza.hpp"

namespace plazza::ipc {

class PizzaPipe {
public:
    PizzaPipe() {
        if (pipe(shellToKitchen) == -1 || pipe(kitchenToShell) == -1)
            throw std::runtime_error("Failed to create pipes");
    }

    ~PizzaPipe() {
        close(shellToKitchen[0]);
        close(shellToKitchen[1]);
        close(kitchenToShell[0]);
        close(kitchenToShell[1]);
    }

    // Shell sends pizza to kitchen
    void sendPizza(const plazza::core::Pizza& pizza) {
        char buffer[sizeof(plazza::core::Pizza::Type) + sizeof(plazza::core::Pizza::Size)];
        std::memcpy(buffer, &pizza.type, sizeof(plazza::core::Pizza::Type));
        std::memcpy(buffer + sizeof(plazza::core::Pizza::Type), &pizza.size, sizeof(plazza::core::Pizza::Size));
        size_t dataSize = sizeof(buffer);
        if (write(shellToKitchen[1], buffer, dataSize) != static_cast<ssize_t>(dataSize))
            throw std::runtime_error("Failed to write pizza to pipe");
    }

    // Kitchen receives pizza from shell
    plazza::core::Pizza receivePizza() {
        plazza::core::Pizza pizza;
        char buffer[sizeof(plazza::core::Pizza::Type) + sizeof(plazza::core::Pizza::Size)];
        ssize_t bytes = read(shellToKitchen[0], buffer, sizeof(buffer));
        if (bytes != sizeof(buffer))
            throw std::runtime_error("Failed to read pizza from pipe");
        std::memcpy(&pizza.type, buffer, sizeof(plazza::core::Pizza::Type));
        std::memcpy(&pizza.size, buffer + sizeof(plazza::core::Pizza::Type), sizeof(plazza::core::Pizza::Size));
        return pizza;
    }

    // Kitchen sends pizza back
    void sendCookedPizza(const plazza::core::Pizza& pizza) {
        char buffer[sizeof(plazza::core::Pizza::Type) + sizeof(plazza::core::Pizza::Size)];
        std::memcpy(buffer, &pizza.type, sizeof(plazza::core::Pizza::Type));
        std::memcpy(buffer + sizeof(plazza::core::Pizza::Type), &pizza.size, sizeof(plazza::core::Pizza::Size));
        size_t dataSize = sizeof(buffer);
        if (write(kitchenToShell[1], buffer, dataSize) != static_cast<ssize_t>(dataSize))
            throw std::runtime_error("Failed to write cooked pizza to pipe");
    }

    // Shell receives cooked pizza
    plazza::core::Pizza receiveCookedPizza() {
        plazza::core::Pizza pizza;
        char buffer[sizeof(plazza::core::Pizza::Type) + sizeof(plazza::core::Pizza::Size)];
        ssize_t bytes = read(kitchenToShell[0], buffer, sizeof(buffer));
        if (bytes != sizeof(buffer))
            throw std::runtime_error("Failed to read cooked pizza from pipe");
        std::memcpy(&pizza.type, buffer, sizeof(plazza::core::Pizza::Type));
        std::memcpy(&pizza.size, buffer + sizeof(plazza::core::Pizza::Type), sizeof(plazza::core::Pizza::Size));
        return pizza;
    }

    // Close unused ends in each process
    void closeShellEnds() {
        // Close shell's ends in child (kitchen)
        close(shellToKitchen[1]);
        close(kitchenToShell[0]);
    }
    void closeKitchenEnds() {
        // Close kitchen's ends in parent (shell)
        close(shellToKitchen[0]);
        close(kitchenToShell[1]);
    }

    int shellToKitchen[2];   // [0]=read, [1]=write
    int kitchenToShell[2];   // [0]=read, [1]=write
};

} // namespace plazza::ipc
