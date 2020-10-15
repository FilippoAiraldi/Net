#pragma once

#include "net_common.h"

namespace net
{
    template <typename T>
    struct message_header
    {
    public:
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct message
    {
        message_header<T> header{};
#ifdef QUEUE_MSGS_IMPLEMENTATION
        std::queue<uint8_t> body;
#else
        std::vector<uint8_t> body;
#endif

        message()
        {
            this->header.size = this->size();
        };

        message(T id) : message()
        {
            this->header.id = id;
        }

        size_t size() const
        {
            return sizeof(message_header<T>) + body.size(); // Returns whole message size
        }

        friend std::ostream &operator<<(std::ostream &os, const message<T> &msg)
        {
            os << msg.to_string();
            return os;
        }

        template <typename TData>
        friend message<T> &operator<<(message<T> &msg, const TData &data)
        {
            // check data type is trivially copiable
            static_assert(std::is_standard_layout<TData>::value, "Data is too complex for message.");

            // resize and copy into vector
#ifdef QUEUE_MSGS_IMPLEMENTATION
            const uint8_t *bytes = (uint8_t *)&data;
            for (size_t i = 0; i < sizeof(TData); ++i)
                msg.body.push(*(bytes + i));
#else
            size_t current_size = msg.body.size();
            msg.body.resize(current_size + sizeof(TData));
            std::memcpy(msg.body.data() + current_size, &data, sizeof(TData));
#endif

            // update size
            msg.header.size = msg.size();
            return msg;
        }

        template <typename TData>
        friend message<T> &operator>>(message<T> &msg, TData &data)
        {
            // check data type is trivially copiable
            static_assert(std::is_standard_layout<TData>::value, "Data is too complex for message.");

            // check vector has enough bytes to populate data
            if (msg.body.empty() || msg.body.size() < sizeof(TData))
                throw std::invalid_argument("Message body cannot be copied into data: not enough bytes.");

// copy from vector and resize
#ifdef QUEUE_MSGS_IMPLEMENTATION
            uint8_t *bytes = (uint8_t *)&data;
            for (size_t i = 0; i < sizeof(TData); ++i)
            {
                *(bytes + i) = msg.body.front();
                msg.body.pop();
            }
#else
            size_t new_size = msg.body.size() - sizeof(TData);
            std::memcpy(&data, msg.body.data() + new_size, sizeof(TData));
            msg.body.resize(new_size);
#endif

            // update size
            msg.header.size = msg.size();
            return msg;
        }

        virtual std::string to_string() const
        {
            return "Message { Id = " +
                   std::to_string(int(this->header.id)) + ", Size = " +
                   std::to_string(this->header.size) + " }";
        }
    };

    template <typename T>
    struct connection; // forward declaration

    template <typename T>
    struct owned_message : message<T>
    {
        std::shared_ptr<connection<T>> remote = nullptr;

        owned_message() : message<T>() {}

        owned_message(T id) : message<T>(id) {}

        std::string to_string() const override
        {
            std::ostringstream os;
            os << "Message { Id = " << int(this->header.id)
               << ", Size = " << this->header.size
               << ", Remote = " << this->remote << " }";
            return os.str();
        }
    };
} // namespace net
