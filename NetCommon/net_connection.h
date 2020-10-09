#pragma once
#include "net_common.h"
#include "net_concurrent_queue.h"
#include "net_message.h"

namespace net
{
    // inheritance from this class provided a shared pointer to "this"
    // instead of a raw pointer
    template <typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
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
        asio::io_context &m_asio_context;           // context of whole asio
        asio::ip::tcp::socket m_socket;             // unique socket to a remote
        concurrent_queue<message<T>> m_q_out;       // queue of msgs to be sent to remote
        concurrent_queue<owned_message<T>> &m_q_in; // queue of msgs sent by remote
    };
} // namespace net
