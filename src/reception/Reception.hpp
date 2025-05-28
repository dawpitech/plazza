/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Reception.hpp
*/

#pragma once

#include <map>
#include <memory>

#include "core/Pizza.hpp"
#include "IPC/TuyauAPizza.hpp"

namespace plazza
{
    class Reception
    {
        public:
            class KitchenData
            {
                public:
                    int kitchenID;
                    pid_t kitchenPID;
                    std::unique_ptr<ipc::TuyauAPizza> pipe;
                    std::vector<core::Pizza> pendingPizzas;

                    KitchenData(pid_t pid, std::unique_ptr<ipc::TuyauAPizza>&& p, int id);
                    KitchenData(KitchenData&&) noexcept;
                    KitchenData& operator=(KitchenData&&) noexcept;
                    KitchenData(const KitchenData&) = delete;
                    KitchenData& operator=(const KitchenData&) = delete;
            };

            Reception() = delete;
            explicit Reception(const float cookingTimeFactor,
                const unsigned int numberOfCooks,
                const unsigned int refillDelay)
                : _uCOOKING_TIME_FACTOR(cookingTimeFactor)
                , _uNUMBER_OF_COOKS(numberOfCooks)
                , _uREFILL_DELAY(refillDelay) {}
            ~Reception() = default;

            [[nodiscard]] std::vector<int> getKitchenPipeFD() const;

            void orderPizza(core::Pizza pizza);
            void checkForCookedPizzas(int kitchenPipeFD);
            void printKitchenStatus();

            void updateKitchenList();

        private:
            const float _uCOOKING_TIME_FACTOR;
            const unsigned int _uNUMBER_OF_COOKS;
            const unsigned int _uREFILL_DELAY;

            int newKitchenID = 0;
            std::vector<KitchenData> _kitchens;

            KitchenData& getNextKitchen();

            void launchNewKitchen();
    };
}
