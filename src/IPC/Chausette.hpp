/*
** EPITECH PROJECT, 2025
** Plazza
** File description:
** Chausette.hpp
*/

#pragma once

#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class IpcUnixSocket {
    public:
        enum class Mode { Server, Client };

        IpcUnixSocket(const std::string& path, Mode mode)
            : _socket_path(path), _mode(mode), _fd(-1), _running(false)
        {}
        IpcUnixSocket(IpcUnixSocket&&) = default;
        IpcUnixSocket& operator=(IpcUnixSocket&&) = default;
        IpcUnixSocket(const IpcUnixSocket&) = delete;
        IpcUnixSocket& operator=(const IpcUnixSocket&) = delete;

        // Server setup: bind and listen
        void startServer() {
            if (_mode != Mode::Server) throw std::runtime_error("Not in server mode");
            _fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (_fd < 0) throw std::runtime_error("Failed to create socket");

            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, _socket_path.c_str(), sizeof(addr.sun_path) - 1);
            unlink(_socket_path.c_str());
            if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                throw std::runtime_error("Failed to bind socket");
            if (listen(_fd, 8) < 0)
                throw std::runtime_error("Failed to listen");

            _running = true;
            _accept_thread = std::thread([this]() { this->acceptLoop(); });
        }

        // Client setup: connect
        void connectClient() {
            if (_mode != Mode::Client) throw std::runtime_error("Not in client mode");
            _fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (_fd < 0) throw std::runtime_error("Failed to create socket");

            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, _socket_path.c_str(), sizeof(addr.sun_path) - 1);

            if (connect(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                throw std::runtime_error("Failed to connect to socket");
        }

        // Send raw data (buffer, length)
        ssize_t send(const char* buf, size_t len) {
            if (_mode == Mode::Client) {
                return ::send(_fd, buf, len, 0);
            }
            std::lock_guard<std::mutex> lock(_clients_mutex);
            ssize_t total = 0;
            for (int cfd : _clients) {
                ssize_t sent = ::send(cfd, buf, len, 0);
                if (sent > 0) total += sent;
            }
            return total;
        }

        // Receive raw data (buffer, length), returns number of bytes read
        ssize_t receive(char* buf, size_t maxlen) {
            if (_mode == Mode::Client) {
                return ::recv(_fd, buf, maxlen, 0);
            } else {
                std::lock_guard<std::mutex> lock(_clients_mutex);
                for (int cfd : _clients) {
                    ssize_t n = ::recv(cfd, buf, maxlen, MSG_DONTWAIT);
                    if (n > 0) return n;
                }
                return 0; // No data
            }
        }

        // For server: blocks until a client sends raw data
        ssize_t receiveBlocking(char* buf, size_t maxlen) {
            while (_running) {
                std::lock_guard<std::mutex> lock(_clients_mutex);
                for (int cfd : _clients) {
                    ssize_t n = ::recv(cfd, buf, maxlen, 0);
                    if (n > 0) return n;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return 0;
        }

        void closeSocket() {
            _running = false;
            if (_accept_thread.joinable()) _accept_thread.join();
            if (_fd != -1) {
                close(_fd);
                _fd = -1;
            }
            if (_mode == Mode::Server) {
                std::lock_guard<std::mutex> lock(_clients_mutex);
                for (int cfd : _clients) close(cfd);
                _clients.clear();
                unlink(_socket_path.c_str());
            }
        }

        ~IpcUnixSocket() { closeSocket(); }

    private:
        std::string _socket_path;
        Mode _mode;
        int _fd;
        std::vector<int> _clients;
        std::thread _accept_thread;
        std::mutex _clients_mutex;
        std::atomic<bool> _running;

        // Server: accept new clients in background
        void acceptLoop() {
            while (_running) {
                int cfd = accept(_fd, nullptr, nullptr);
                if (cfd >= 0) {
                    std::lock_guard<std::mutex> lock(_clients_mutex);
                    _clients.push_back(cfd);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
};
