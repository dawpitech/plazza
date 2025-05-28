/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Pizza.cpp
*/

#include <cstring>

#include "Pizza.hpp"

std::string plazza::core::Pizza::getTypeName(const Type type)
{
    switch (type) {
        case Type::Regina: return "Regina";
        case Type::Margarita: return "Margarita";
        case Type::Americana: return "Americana";
        case Type::Fantasia: return "Fantasia";
        default: throw std::exception();
    }
}

std::string plazza::core::Pizza::getSizeName(const Size size)
{
    switch (size) {
        case Size::S: return "S";
        case Size::M: return "M";
        case Size::L: return "L";
        case Size::XL: return "XL";
        case Size::XXL: return "XXL";
        default: throw std::exception();
    }
}

plazza::core::Pizza::Type plazza::core::Pizza::getType(const std::string& type)
{
    if (type == "Regina")
        return Type::Regina;
    if (type == "Margarita")
        return Type::Margarita;
    if (type == "Americana")
        return Type::Americana;
    if (type == "Fantasia")
        return Type::Fantasia;
    throw std::out_of_range("Pizza type " + type + " unknown");
}

plazza::core::Pizza::Size plazza::core::Pizza::getSize(const std::string& size)
{
    if (size == "S")
        return Size::S;
    if (size == "M")
        return Size::M;
    if (size == "L")
        return Size::L;
    if (size == "XL")
        return Size::XL;
    if (size == "XXL")
        return Size::XXL;
    throw std::out_of_range("Pizza size " + size + " unknown");
}

const void* plazza::core::Pizza::pack() const { return this; }
void plazza::core::Pizza::unpack(const void* data) { std::memcpy(this, data, sizeof(Pizza)); }

std::chrono::duration<long, std::ratio<1, 1000>> plazza::core::Pizza::getCookingTime() const
{
    switch (this->type)
    {
        case Type::Regina: return std::chrono::seconds(1);
        case Type::Margarita: return std::chrono::seconds(2);
        case Type::Americana: return std::chrono::seconds(2);
        case Type::Fantasia: return std::chrono::seconds(4);
        default: throw std::out_of_range("Invalid pizza type");
    }
}
