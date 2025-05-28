/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** TuyauAPizza.hpp
*/

#pragma once

#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include "core/Pizza.hpp"

namespace plazza::ipc
{
    class TuyauAPizza
        {
        public:
            class KitchenClosedException final : public std::runtime_error
            {
                public:
                    KitchenClosedException() : std::runtime_error("The kitchen has closed") {}
            };

            TuyauAPizza() {
                if (pipe(shellToKitchen) == -1 || pipe(kitchenToShell) == -1)
                    throw std::runtime_error("Failed to create pipes");
            }

            ~TuyauAPizza() {
                close(shellToKitchen[0]);
                close(shellToKitchen[1]);
                close(kitchenToShell[0]);
                close(kitchenToShell[1]);
            }

            void sendPizza(const core::Pizza& pizza) const
            {
                char buffer[sizeof(core::Pizza::Type) + sizeof(core::Pizza::Size)];
                std::memcpy(buffer, &pizza.type, sizeof(core::Pizza::Type));
                std::memcpy(buffer + sizeof(core::Pizza::Type), &pizza.size, sizeof(core::Pizza::Size));
                // ReSharper disable once CppTooWideScopeInitStatement
                constexpr size_t dataSize = sizeof(buffer);
                if (write(shellToKitchen[1], buffer, dataSize) != static_cast<ssize_t>(dataSize))
                    throw std::runtime_error("Failed to write pizza to pipe");
            }

            [[nodiscard]] core::Pizza receivePizza() const
            {
                core::Pizza pizza{};
                char buffer[sizeof(core::Pizza::Type) + sizeof(core::Pizza::Size)];
                // ReSharper disable once CppTooWideScopeInitStatement
                const ssize_t bytes = read(shellToKitchen[0], buffer, sizeof(buffer));
                if (bytes != sizeof(buffer))
                    throw std::runtime_error("Failed to read pizza from pipe");
                std::memcpy(&pizza.type, buffer, sizeof(core::Pizza::Type));
                std::memcpy(&pizza.size, buffer + sizeof(core::Pizza::Type), sizeof(core::Pizza::Size));
                return pizza;
            }

            void sendCookedPizza(const core::Pizza& pizza) const
            {
                char buffer[sizeof(core::Pizza::Type) + sizeof(core::Pizza::Size)];
                std::memcpy(buffer, &pizza.type, sizeof(core::Pizza::Type));
                std::memcpy(buffer + sizeof(core::Pizza::Type), &pizza.size, sizeof(core::Pizza::Size));
                // ReSharper disable once CppTooWideScopeInitStatement
                constexpr size_t dataSize = sizeof(buffer);
                if (write(kitchenToShell[1], buffer, dataSize) != static_cast<ssize_t>(dataSize))
                    throw std::runtime_error("Failed to write cooked pizza to pipe");
            }

            [[nodiscard]] core::Pizza receiveCookedPizza() const
            {
                core::Pizza pizza{};
                char buffer[sizeof(core::Pizza::Type) + sizeof(core::Pizza::Size)];
                const ssize_t bytes = read(kitchenToShell[0], buffer, sizeof(buffer));
                if (bytes == 0)
                    throw KitchenClosedException();
                if (bytes != sizeof(buffer))
                    throw std::runtime_error("Failed to read cooked pizza from pipe");
                std::memcpy(&pizza.type, buffer, sizeof(core::Pizza::Type));
                std::memcpy(&pizza.size, buffer + sizeof(core::Pizza::Type), sizeof(core::Pizza::Size));
                return pizza;
            }

            // ReSharper disable once CppMemberFunctionMayBeConst
            void closeReceptionSidePipes() {
                close(shellToKitchen[1]);
                close(kitchenToShell[0]);
            }
            // ReSharper disable once CppMemberFunctionMayBeConst
            void closeKitchenSidePipes() {
                close(shellToKitchen[0]);
                close(kitchenToShell[1]);
            }

            int shellToKitchen[2] = {-1, -1};
            int kitchenToShell[2] = {-1, -1};
    };
}
