#pragma once

#include "net_common.h"
#include "net_concurrent_queue.h"
#include "net_message.h"
#include "net_connection.h"

namespace net
{
    template <typename T>
    class client_interface
    {
    public:
        client_interface() : socket(context) {}
        virtual ~client_interface() { disconnect(); }

    public:
        bool connect(const std::string &host, const uint16_t port)
        {
            try
            {
                // create connection
                this->connectionToServer = std::make_unique<connection<T>>(); // todo

                // resolve hostname/ip-address into physical address
                asio::ip::tcp::resolver resolver(this->context);
                auto m_endpoints = resolver.resolve(host, std::to_string(port));

                this->connectionToServer->connectToServer(); // todo
                this->contextThread = std::thread([this]() { this->context.run(); });
            }
            catch (const std::exception &e)
            {
                std::cerr << "[CLIENT] Exception: " << e.what() << '\n';
                throw e;
            }

            return false;
        }

        void disconnect()
        {
            if (isConnected())
                this->connectionToServer->disconnect();

            this->context.stop();
            if (this->contextThread.joinable())
                this->contextThread.join();

            this->connectionToServer.release();
        }

        bool isConnected()
        {
            if (this->connectionToServer)
                return this->connectionToServer->isConnected();
            return false;
        }

        void send(const message<T> &msg) {}

        concurrent_queue<owned_message<T>> &getIncomingMessages() { return this->msgsIn; }

    protected:
        asio::io_context context;                          // handles data transfer
        std::thread contextThread;                         // thread for the asio context
        asio::ip::tcp::socket socket;                      // socket to server
        std::unique_ptr<connection<T>> connectionToServer; // client instance of connection to server

    private:
        concurrent_queue<owned_message<T>> msgsIn; // incoming server msgs
    };
} // namespace net
