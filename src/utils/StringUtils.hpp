/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** StringUtils.hpp
*/

#pragma once

#include <string>
#include <vector>

namespace plazza::utils
{
    class StringUtils
    {
        public:
            static std::string applyCamelCase(const std::string& str)
            {
                std::string result = str;
                bool newWord = true;

                for (size_t i = 0; i < result.length(); ++i) {
                    if (newWord && std::isalpha(result[i])) {
                        result[i] = static_cast<char>(std::toupper(result[i]));
                        newWord = false;
                    } else if (result[i] == ' ') {
                        newWord = true;
                    } else {
                        result[i] = static_cast<char>(std::tolower(result[i]));
                    }
                }
                return result;
            }
            static std::vector<std::string> split(const std::string& str, const char delimiter)
            {
                std::vector<std::string> tokens;
                std::size_t start = 0, end = 0;

                while ((end = str.find(delimiter, start)) != std::string::npos) {
                    tokens.push_back(str.substr(start, end - start));
                    start = end + 1;
                }
                tokens.push_back(str.substr(start));
                return tokens;
            }
    };
}
