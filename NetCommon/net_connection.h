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
        connection() {}

        virtual ~connection() {}

    public:
        bool connectToServer();
        bool disconnect();
        bool isConnected() const;

    public:
        bool send(const message<T> &msg);

    protected:
        asio::io_context &context;                  // context of whole asio
        asio::ip::tcp::socket socket;               // unique socket to a remote
        concurrent_queue<message<T>> msgsOut;       // queue of msgs to be sent to remote
        concurrent_queue<owned_message<T>> &msgsIn; // queue of msgs sent by remote
    };
} // namespace net
