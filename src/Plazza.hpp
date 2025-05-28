#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include "IPC/TuyauAPizza.hpp"
#include "reception/Reception.hpp"

namespace plazza
{
    class Plazza
    {
        public:
            using CLICommand = void (*)(Plazza& plazza);

            Plazza() = default;
            ~Plazza() = default;

            int parseLaunchArgs(int argc, const char **argv);
            void run();

        private:
            enum class ORDER_COMMAND_STATE {
                VALID_ORDER,
                INVALID_ORDER,
                NOT_AN_ORDER,
            };

            float cookingTimeFactor = 0.f;
            unsigned int cooksPerKitchen = 0;
            unsigned int refillDelay = 0;
            bool running = true;

            std::unique_ptr<Reception> _reception;

            void runStatus(const Plazza& plazza) const;
            void runQuit(Plazza& plazza);
            ORDER_COMMAND_STATE parsePizzaOrder(Plazza& plazza, const std::string& rawOrder);

            static void initChildWatcher();

            // ReSharper disable CppParameterMayBeConstPtrOrRef
            std::map<std::string, CLICommand> STATIC_COMMANDS = {
                {"status", [](Plazza& plazza) { plazza.runStatus(plazza); }},
                {"quit", [](Plazza& plazza) { plazza.runQuit(plazza); }},
                {"exit", [](Plazza& plazza) { plazza.runQuit(plazza); }},
            };
            // ReSharper restore CppParameterMayBeConstPtrOrRef

            void runCLI();
    };
}
