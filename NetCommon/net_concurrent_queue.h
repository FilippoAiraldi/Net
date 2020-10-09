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
            std::scoped_lock lock(m_mutex);
            return m_q.front();
        }

        const T &back()
        {
            std::scoped_lock lock(m_mutex);
            return m_q.back();
        }

        void push_back(const T &item)
        {
            std::scoped_lock lock(m_mutex);
            m_q.push_back(std::move(item));
        }

        void push_front(const T &item)
        {
            std::scoped_lock lock(m_mutex);
            m_q.push_front(std::move(item));
        }

        bool empty()
        {
            std::scoped_lock lock(m_mutex);
            return m_q.empty();
        }

        size_t size()
        {
            std::scoped_lock lock(m_mutex);
            return m_q.size();
        }

        void clear()
        {
            std::scoped_lock lock(m_mutex);
            m_q.clear();
        }

        T pop_front()
        {
            std::scoped_lock lock(m_mutex);
            T item = std::move(m_q.front());
            m_q.pop_front();
            return item;
        }

        T pop_back()
        {
            std::scoped_lock lock(m_mutex);
            T item = std::move(m_q.back());
            m_q.pop_back();
            return item;
        }

    private:
        std::mutex m_mutex;
        std::deque<T> m_q;
    };
} // namespace net
