/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Kitchen.hpp
*/

#pragma once
#include <mutex>

namespace plazza::kitchen
{
    class Kitchen
    {
        public:
            Kitchen() = delete;
            explicit Kitchen(const int id) : _id(id) {}
            ~Kitchen() = default;

            void processEntryPoint();

        private:
            int _id;
    };
}
