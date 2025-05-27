/*
** EPITECH PROJECT, 2025
** plazza
** File description:
** main.cpp
*/

#include "Plazza.hpp"

#include "core/Pizza.hpp"

int main(const int argc, const char **argv) {
    plazza::Plazza plazza{};

    if (plazza.parseLaunchArgs(argc, argv) != 0)
        return 84;
    plazza.run();
    return 0;
}
