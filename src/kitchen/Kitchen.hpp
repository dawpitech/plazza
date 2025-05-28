/*
** EPITECH PROJECT, 2025
** Kitchen
** File description:
** Kitchen.hpp
*/

#pragma once

#include <memory>
#include <mutex>

#include "IPC/TuyauAPizza.hpp"

namespace plazza::kitchen
{
    class Kitchen
    {
        public:
            Kitchen() = delete;
            Kitchen(const int id, std::unique_ptr<ipc::TuyauAPizza> p, const float cookingTimeFactor)
                : _id(id), pipe(std::move(p)), _cookingTimeFactor(cookingTimeFactor) {}
            ~Kitchen() = default;

            void processEntryPoint();

        private:
            int _id;
            std::unique_ptr<ipc::TuyauAPizza> pipe;
            float _cookingTimeFactor;
    };
}
