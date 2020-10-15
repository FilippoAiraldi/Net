#pragma once

#include "net_common.h"

namespace net
{
    template <typename T>
    class concurrent_queue
    {
    public:
        concurrent_queue() = default;
        concurrent_queue(const concurrent_queue<T> &) = delete; // prevent copying since it has mutexes

        virtual ~concurrent_queue() { clear(); }

        const T &front()
        {
            std::scoped_lock lock(this->mtx);
            return this->dq.front();
        }

        const T &back()
        {
            std::scoped_lock lock(this->mtx);
            return this->dq.back();
        }

        void push_back(const T &item)
        {
            std::scoped_lock lock(this->mtx);
            this->dq.push_back(std::move(item));
        }

        void push_front(const T &item)
        {
            std::scoped_lock lock(this->mtx);
            this->dq.push_front(std::move(item));
        }

        bool empty()
        {
            std::scoped_lock lock(this->mtx);
            return this->dq.empty();
        }

        size_t size()
        {
            std::scoped_lock lock(this->mtx);
            return this->dq.size();
        }

        void clear()
        {
            std::scoped_lock lock(this->mtx);
            this->dq.clear();
        }

        T pop_front()
        {
            std::scoped_lock lock(this->mtx);
            T item = std::move(this->dq.front());
            this->dq.pop_front();
            return item;
        }

        T pop_back()
        {
            std::scoped_lock lock(this->mtx);
            T item = std::move(this->dq.back());
            this->dq.pop_back();
            return item;
        }

    private:
        std::mutex mtx;
        std::deque<T> dq;
    };
} // namespace net
