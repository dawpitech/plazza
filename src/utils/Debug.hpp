/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Debug.hpp
*/

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace plazza::debug
{
    extern bool DEBUG_MODE;

    static void printTS()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::cout << "(" << std::put_time(std::localtime(&t), "%M:%S") << "." << std::setfill('0') << std::setw(3)
            << (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000).count() << ") ";
    }
    inline std::string getTS()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << RESET << BOLDMAGENTA
            << "(" << std::put_time(std::localtime(&t), "%M:%S") << "."
            << std::setfill('0') << std::setw(3)
            << (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000).count()
            << ") "
            << RESET;
        return oss.str();
    }
}
