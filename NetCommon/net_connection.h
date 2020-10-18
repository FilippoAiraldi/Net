#pragma once

#include "net_common.h"
#include "net_concurrent_queue.h"
#include "net_message.h"

namespace net
{
    template <typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
        // inheritance from std::enable_shared_from_this class
        // provides a shared pointer to "this", instead of a raw pointer

    public:
        enum class owner
        {
            server,
            client
        };

        connection(owner owner, asio::io_context &newContext, asio::ip::tcp::socket newSocket, concurrent_queue<owned_message<T>> &queue)
            : context(newContext), socket(std::move(newSocket)), msgsIn(queue)
        {
            this->ownerType = owner;
        }

        virtual ~connection() {}

        void connectToThisClient(uint32_t id)
        {
            if (this->ownerType != owner::server)
                return;
            if (this->socket.is_open())
            {
                this->id = id;
                this->readHeaderAsync();
            }
        }

        void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints)
        {
            if (this->ownerType != owner::client)
                return;

            asio::async_connect(
                this->socket,
                endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint ep) {
                    if (!ec)
                        this->readHeaderAsync();
                });
        }

        void disconnect()
        {
            if (this->isConnected())
                asio::post(this->context, [this]() { this->socket.close(); });
        }

        bool isConnected() const { return this->socket.is_open(); }
        uint32_t getId() const { return this->id; }

        void send(const message<T> &msg)
        {
            asio::post(
                this->context,
                [this, msg]() {
                    bool writingMsg = !this->msgsOut.empty();
                    this->msgsOut.push_back(msg);

                    if (!writingMsg)
                        this->writeHeaderAsync();
                });
        }

        friend std::ostream &operator<<(std::ostream &os, const connection<T> &conn)
        {
            os << conn.socket.remote_endpoint() << " (" << conn.id << ")";
            return os;
        }

    protected:
        asio::io_context &context;                  // context of whole asio
        asio::ip::tcp::socket socket;               // unique socket to a remote
        concurrent_queue<message<T>> msgsOut;       // queue of msgs to be sent to remote
        concurrent_queue<owned_message<T>> &msgsIn; // queue of msgs sent by remote

        message<T> tmpMsg;

        owner ownerType = owner::server;
        uint32_t id = 0;

    private:
        void readHeaderAsync()
        {
            asio::async_read(
                this->socket,
                asio::buffer(&this->tmpMsg.header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec)
                    {
                        if (this->tmpMsg.header.size > 0)
                        {
                            this->tmpMsg.resizeBody(tmpMsg.header.size);
                            this->readBodyAsync();
                        }
                        else
                        {
                            this->addToIncomingMessageQueue();
                        }
                    }
                    else
                    {
                        std::cout << "[CONNECTION " << this->id << "] Read header failed.\n";
                        this->socket.close();
                    }
                });
        }

        void readBodyAsync()
        {
            asio::async_read(
                this->socket,
                asio::buffer(this->tmpMsg.body.data(), this->tmpMsg.body.size()),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec)
                    {
                        this->addToIncomingMessageQueue();
                    }
                    else
                    {
                        std::cout << "[CONNECTION " << this->id << "] Read body failed.\n";
                        this->socket.close();
                    }
                });
        }

        void writeHeaderAsync()
        {
            asio::async_write(
                this->socket,
                asio::buffer(&this->msgsOut.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec)
                    {
                        if (this->msgsOut.front().body.size() > 0)
                        {
                            this->writeBodyAsync();
                        }
                        else
                        {
                            this->msgsOut.pop_front();

                            if (!this->msgsOut.empty())
                                this->writeHeaderAsync();
                        }
                    }
                    else
                    {
                        std::cout << "[CONNECTION " << this->id << "] Write header failed.\n";
                        this->socket.close();
                    }
                });
        }

        void writeBodyAsync()
        {
            asio::async_write(
                this->socket,
                asio::buffer(this->msgsOut.front().body.data(), this->msgsOut.front().body.size()),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec)
                    {
                        this->msgsOut.pop_front();
                        if (!this->msgsOut.empty())
                            this->writeHeaderAsync();
                    }
                    else
                    {
                        std::cout << "[CONNECTION " << this->id << "] Write body failed.\n";
                        this->socket.close();
                    }
                });
        }

        void addToIncomingMessageQueue()
        {
            owned_message<T> owned_msg;
            owned_msg.msg = this->tmpMsg;

            if (this->ownerType == owner::server)
                owned_msg.remote = this->shared_from_this();

            this->msgsIn.push_back(owned_msg);

            this->readHeaderAsync();
        }
    };
} // namespace net
