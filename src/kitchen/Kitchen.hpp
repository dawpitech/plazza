/*
** EPITECH PROJECT, 2025
** Kitchen
** File description:
** Kitchen.hpp
*/

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "cook/Cook.hpp"
#include "IPC/TuyauAPizza.hpp"
#include "utils/ThreadSafeQueue.hpp"

namespace plazza::kitchen
{
    class Kitchen
    {
        public:
            Kitchen() = delete;
            Kitchen(int id, std::unique_ptr<ipc::TuyauAPizza> p,
                float cookingTimeFactor,
                unsigned int numberOfCooks,
                unsigned int refillDelay);

            ~Kitchen() = default;

            void processEntryPoint();

        private:
            static constexpr unsigned int TIMEOUT = 5;

            const int _id;
            const std::unique_ptr<ipc::TuyauAPizza> pipe;
            const float _cookingTimeFactor;
            const unsigned int _numberOfCooks;
            const unsigned int _refillDelay;

            std::vector<std::unique_ptr<Cook>> _cooks;
            utils::ThreadSafeQueue<core::Pizza> _waitingOrders;
            utils::ThreadSafeQueue<core::Pizza> _finishedOrders;
            std::unordered_map<core::Ingredients, unsigned int> _ingredientStock;
            std::mutex _ingredientMutex;
    };
}
