/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** ThreadSafeQueue.hpp
*/

#pragma once

#include <mutex>
#include <optional>
#include <queue>

namespace plazza::utils
{
    template <typename K>
    class ThreadSafeQueue
    {
        public:
            ThreadSafeQueue() = default;
            ~ThreadSafeQueue() = default;

            bool empty()
            {
                std::lock_guard guard(this->_accessMtx);
                return this->_queue.empty();
            }
            [[nodiscard, deprecated("ThreadSafeQueue#maybeGet() is recommended")]] K get()
            {
                std::lock_guard guard(this->_accessMtx);
                if (this->_queue.empty())
                    throw std::runtime_error("Get on an empty thread-safe queue");
                auto object = this->_queue.front();
                this->_queue.pop();
                return object;
            }
            void push(const K object)
            {
                std::lock_guard guard(this->_accessMtx);
                this->_queue.push(object);
            }

            std::optional<core::Pizza> maybeGet()
            {
                std::lock_guard guard(this->_accessMtx);
                if (this->_queue.empty())
                    return std::nullopt;
                auto object = this->_queue.front();
                this->_queue.pop();
                return object;
            }

        private:
            std::queue<K> _queue;
            std::mutex _accessMtx;
    };
}
