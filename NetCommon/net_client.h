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
        client_interface() : m_socket(m_context) {}
        virtual ~client_interface() { disconnect(); }

    public:
        bool connect(const std::string &host, const uint16_t port)
        {
            try
            {
                // create connection
                m_connection = std::make_unique<connection<T>>(); // todo

                // resolve hostname/ip-address into physical address
                asio::ip::tcp::resolver resolver(m_context);
                auto m_endpoints = resolver.resolve(host, std::to_string(port));

                m_connection->connectToServer(); // todo
                m_context_thread = std::thread([this]() { m_context.run(); });
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }

            return false;
        }

        void disconnect()
        {
            if (isConnected())
                m_connection->disconnect();

            m_context.stop();
            if (m_context_thread.joinable())
                m_context_thread.join();

            m_connection.release();
        }

        bool isConnected()
        {
            if (m_connection)
                return m_connection->isConnected();
            return false;
        }

        void send(const message<T> &msg) {}

        concurrent_queue<owned_message<T>> &getIncomingMessages() { return m_q_in; }

    protected:
        asio::io_context m_context;                  // handles data transfer
        std::thread m_context_thread;                // thread for the asio context
        asio::ip::tcp::socket m_socket;              // socket to server
        std::unique_ptr<connection<T>> m_connection; // client instance of connection to server

    private:
        concurrent_queue<owned_message<T>> m_q_in; // incoming server msgs
    };
} // namespace net
