#pragma once
#include <mutex>
#include <memory>
#include "IPC/PizzaPipe.hpp"

namespace plazza::kitchen
{
    class Kitchen
    {
        public:
            Kitchen() = delete;
            Kitchen(const int id, std::unique_ptr<plazza::ipc::PizzaPipe> p)
                : _id(id), pipe(std::move(p)) {}
            ~Kitchen() = default;

            void processEntryPoint();

        private:
            int _id;
            std::unique_ptr<plazza::ipc::PizzaPipe> pipe;
    };
}
