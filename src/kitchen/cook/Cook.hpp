/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Cook.hpp
*/

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "core/Ingredients.hpp"
#include "core/Pizza.hpp"
#include "utils/ThreadSafeQueue.hpp"

namespace plazza::kitchen
{
    class Cook
    {
        public:
            Cook() = delete;
            explicit Cook(std::unordered_map<core::Ingredients, unsigned int>& ingredientStock,
                          std::mutex& ingredientMtx,
                          utils::ThreadSafeQueue<core::Pizza>& waitingOrders,
                          utils::ThreadSafeQueue<core::Pizza>& finishedOrders);
            ~Cook() = default;

            void letHimCook();

        private:
            std::unique_ptr<std::thread> _thread;
            std::unordered_map<core::Ingredients, unsigned int>& _ingredientStock;
            std::mutex& _ingredientMutex;
            utils::ThreadSafeQueue<core::Pizza>& _waitingOrders;
            utils::ThreadSafeQueue<core::Pizza>& _finishedOrders;

            [[noreturn]] static void threadEntryPoint(Cook& self);
    };
}
