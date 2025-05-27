/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Ingredients.hpp
*/

#pragma once

namespace plazza::core
{
    enum class Ingredients
    {
        Dough = 1 << 0,
        Tomato = 1 << 1,
        Gruyere = 1 << 2,
        Ham = 1 << 3,
        Mushrooms = 1 << 4,
        Steak = 1 << 5,
        Eggplant = 1 << 6,
        GoatCheese = 1 << 7,
        ChiefLove = 1 << 8,
    };
}
