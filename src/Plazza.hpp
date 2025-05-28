#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include "IPC/PizzaPipe.hpp"

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

            void runStatus(const Plazza& plazza);
            void runQuit(Plazza& plazza);
            ORDER_COMMAND_STATE parsePizzaOrder(Plazza& plazza, const std::string& rawOrder);

            void updateKitchenList();
            static void initChildWatcher();
            void addNewKitchen();

            std::map<std::string, CLICommand> STATIC_COMMANDS = {
                {"status", [](Plazza& plazza) { plazza.runStatus(plazza); }},
                {"quit", [](Plazza& plazza) { plazza.runQuit(plazza); }},
                {"exit", [](Plazza& plazza) { plazza.runQuit(plazza); }},
            };

            int kitchenID = 0;

            class KitchenData
            {
                public:
                    pid_t kitchenPID;
                    std::unique_ptr<plazza::ipc::PizzaPipe> pipe;

                    KitchenData(pid_t pid, std::unique_ptr<plazza::ipc::PizzaPipe>&& p);
                    // Move only
                    KitchenData(KitchenData&&) noexcept;
                    KitchenData& operator=(KitchenData&&) noexcept;
                    KitchenData(const KitchenData&) = delete;
                    KitchenData& operator=(const KitchenData&) = delete;
            };

            std::vector<KitchenData> _kitchens;

            // Each pizza being cooked
            struct PendingPizza {
                pid_t kitchenPID;
                core::Pizza pizza;
            };
            // For each kitchen, the pizzas it's currently cooking
            std::map<pid_t, std::vector<core::Pizza>> _pendingPizzasPerKitchen;

            void checkForCookedPizzas();

            void runCLI();
    };
}
