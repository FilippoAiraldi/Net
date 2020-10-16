#pragma once

#include "net_common.h"
#include "net_concurrent_queue.h"
#include "net_message.h"
#include "net_connection.h"

namespace net
{
    template <typename T>
    class server_interface
    {
    public:
        server_interface(uint16_t port) : acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        {
        }

        virtual ~server_interface()
        {
            this->stop();
        }

        bool start()
        {
            try
            {
                this->WaitForClientConnectionAsync();
                this->context = std::thread([this]() { this->context.run(); });
            }
            catch (const std::exception &e)
            {
                std::cerr << "[SERVER] Exception: " << e.what() << '\n';
                return false;
            }

            std::cout << "[SERVER] Started.\n";
            return true;
        }

        bool stop()
        {
            this->context.stop();
            if (this->contextThread.joinable())
                this->contextThread.join();

            std::cout << "[SERVER] Stopped.\n";
        }

        void waitForClientConnectionAsync()
        {
            this->acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec)
                {
                    std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << "\n";

                    std::shared_ptr<connect<T>> newConnection = std::make_shared<connection<T>>(
                        connection<T>::owner::server,
                        this->context,
                        std::move(socket),
                        this->msgsIn);

                    if (this->onClientConnecting(newConnection))
                    {
                        this->connections.push_back(std::move(newConnection));
                        this->connections.back()->connectToClient(this->idCounter++);

                        std::cout << "[SERVER] Connection " << socket.remote_endpoint()
                                  << "approved with id: " << this->connections.back()->id() << ".\n";
                    }
                    else
                    {
                        std::cout << "[SERVER] Connection " << socket.remote_endpoint() << "denied.\n";
                    }
                }
                else
                {
                    std::cerr << "[SERVER] New connection error: " << ec.message() << "\n";
                }

                this->waitForClientConnectionAsync();
            });
        }

        void sendMessage(std::shared_ptr<connection<T>> client, const message<T> &msg)
        {
            if (client && client->isConnected())
            {
                client->send(msg);
            }
            else
            {
                this->onClientDisconnected(client);
                client.reset();
                this->removeConnection(client);
            }
        }

        void sendMessageToAllClients(const message<T> &msg, std::shared_ptr<connection<T>> clientToIgnore = nullptr)
        {
            bool anyInvalidClients = false;

            for (std::shared_ptr<connection<T>> &client : this->connections)
            {
                if (client && client->isConnected())
                {
                    if (client != clientToIgnore)
                        client->send(msg);
                }
                else
                {
                    this->onClientDisconnected(client);
                    client.reset();
                    anyInvalidClients = true;
                }
            }

            if (anyInvalidClients)
                this->removeConnection(nullptr);
        }

        void Update(size_t maxMessages = -1)
        {
            size_t msgsCnt = 0;
            while (msgsCnt < maxMessages && !this->msgsIn.empty())
            {
                owned_message<T> msg = this->msgsIn.pop_front();
                this->onMessage(msg.remote, msg);

                msgsCnt++;
            }
        }

    protected:
        virtual bool onClientConnecting(std::shared_ptr<connection<T>> client)
        {
            return false;
        }

        virtual void onClientDisconnected(std::shared_ptr<connection<T>> client)
        {
        }

        virtual void onMessage(std::shared_ptr<connection<T>> client, message<T> &msg)
        {
        }

    protected:
        concurrent_queue<owned_message<T>> msgsIn; // thread-safe incoming msgs queue
        asio::io_context context;                  // for running asio stuff
        std::thread contextThread;                 // required by the asio context

        std::deque<std::shared_ptr<connection<T>>> connections; // deque of all client connections

        asio::ip::tcp::acceptor acceptor;
        uint32_t idCounter = 10000;

    private:
        void removeConnection(std::shared_ptr<connection<T>> client)
        {
            this->connections.erase(
                std::remove(this->connections.begin(), this->connections.end(), client),
                this->connections.end());
        }
    };
} // namespace net
