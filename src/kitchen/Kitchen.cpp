/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Kitchen.cpp
*/

#include <iostream>
#include <bits/this_thread_sleep.h>

#include "Kitchen.hpp"
#include "core/Pizza.hpp"
#include "IPC/Chausette.hpp"

void plazza::kitchen::Kitchen::processEntryPoint()
{
    std::cout << "[K" << this->_id << "] Process entry" << std::endl;

    IpcUnixSocket socket("/tmp/plazza" + this->_id, IpcUnixSocket::Mode::Client);

    char buf[sizeof(int32_t) * 2];
    const ssize_t n = socket.receiveBlocking(buf, sizeof(buf));
    if (n == sizeof(buf)) {
        core::Pizza pizza;
        pizza.unpack(buf);

        std::cout << "Received pizza" << pizza << std::endl;
    }

    std::cout << "[K" << this->_id << "] Process returning" << std::endl;
}
