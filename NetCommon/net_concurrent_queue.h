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

    public:
        const T &front()
        {
            std::scoped_lock lock(this->mtxQueue);
            return this->dq.front();
        }

        const T &back()
        {
            std::scoped_lock lock(this->mtxQueue);
            return this->dq.back();
        }

        T pop_front()
        {
            std::scoped_lock lock(this->mtxQueue);
            T item = std::move(this->dq.front());
            this->dq.pop_front();
            return item;
        }

        T pop_back()
        {
            std::scoped_lock lock(this->mtxQueue);
            T item = std::move(this->dq.back());
            this->dq.pop_back();
            return item;
        }

        void push_back(const T &item)
        {
            std::scoped_lock lock(this->mtxQueue);
            this->dq.push_back(std::move(item));

            std::unique_lock<std::mutex> ul(this->mtxBlock);
            this->cv.notify_one();
        }

        void push_front(const T &item)
        {
            std::scoped_lock lock(this->mtxQueue);
            this->dq.push_front(std::move(item));

            std::unique_lock<std::mutex> ul(this->mtxBlock);
            this->cv.notify_one();
        }

        bool empty()
        {
            std::scoped_lock lock(this->mtxQueue);
            return this->dq.empty();
        }

        size_t size()
        {
            std::scoped_lock lock(this->mtxQueue);
            return this->dq.size();
        }

        void clear()
        {
            std::scoped_lock lock(this->mtxQueue);
            this->dq.clear();
        }

        void wait()
        {
            while (this->empty())
            {
                std::unique_lock<std::mutex> ul(this->mtxBlock);
                this->cv.wait(ul);
            }
        }

    protected:
        std::mutex mtxQueue;
        std::deque<T> dq;

        std::condition_variable cv;
        std::mutex mtxBlock;
    };
} // namespace net
