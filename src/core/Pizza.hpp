/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Pizza.hpp
*/

#pragma once

#include <chrono>
#include <ostream>
#include <vector>

#include "Ingredients.hpp"

namespace plazza::core
{
    class Pizza
    {
        public:
            enum class Type
            {
                Regina = 1 << 0,
                Margarita = 1 << 1,
                Americana = 1 << 2,
                Fantasia = 1 << 3,
            };

            enum class Size
            {
                S = 1 << 0,
                M = 1 << 1,
                L = 1 << 2,
                XL = 1 << 3,
                XXL = 1 << 4,
            };

            Type type;
            Size size;

            [[nodiscard]] std::vector<Ingredients> getIngredients() const;
            [[nodiscard]] std::chrono::duration<long, std::ratio<1, 1000>> getCookingTime() const;

            [[nodiscard]] static auto getTypeName(Type type) -> std::string;
            [[nodiscard]] static std::string getSizeName(Size size);

            [[nodiscard]] static Type getType(const std::string& type);
            [[nodiscard]] static Size getSize(const std::string& size);

            [[nodiscard]] const void* pack() const;
            void unpack(const void* data);
    };
}

inline std::ostream& operator<<(std::ostream& os, const plazza::core::Pizza& pizza)
{
    os << plazza::core::Pizza::getTypeName(pizza.type) << " " << plazza::core::Pizza::getSizeName(pizza.size);
    return os;
}
